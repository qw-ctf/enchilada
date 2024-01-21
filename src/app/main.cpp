#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSortFilterProxyModel>
#include <QSGRendererInterface>

#include <QFile>
#include <QDataStream>
#include <QDebug>

#include <wad.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>

#include "TextureItemModel.h"
#include "WadImageProvider.h"

QVector<dmiptex_t> parseWadFile(QFile &file) {
	QVector<dmiptex_t> textures;

	QDataStream in(&file);

	in.setByteOrder(QDataStream::LittleEndian);

	wadhead_t wadHeader = {0};
	in.readRawData(wadHeader.magic, 4);
	in >> wadHeader.numentries;
	in >> wadHeader.diroffset;

	if (strncmp(wadHeader.magic, "WAD2", 4) != 0) {
		qDebug() << "Error: Invalid WAD file";
		return textures;
	}

	qDebug() << "numentries: " << wadHeader.numentries;

	if (!file.seek(wadHeader.diroffset)) {
		qDebug() << "Failed to seek to diroffset: " << wadHeader.diroffset;
		return textures;
	}

	QVector<wadentry_t> entries(wadHeader.numentries);
	for (int i = 0; i < wadHeader.numentries; ++i) {
		wadentry_t &entry = entries[i];
		in >> entry.offset;
		in >> entry.dsize;
		in >> entry.size;
		in >> entry.type;
		in >> entry.cmprs;
		in >> entry.dummy;
		in.readRawData(entry.name, 16);
	}

	for (const wadentry_t &entry: entries) {
		file.seek(entry.offset);

		switch (entry.type) {
			case '@': // Color Palette
				break;
			case 'B': // Pictures for status bar
				break;
			case 'D': // Mip Texture
				dmiptex_t mip;

				in.readRawData(mip.name, 16);
				in >> mip.width;
				in >> mip.height;
				for (int i = 0; i < 4; i++) {
					in >> mip.offsets[i];
					mip.offsets[i] += entry.offset;
				}
				{
					bool erase = false;
					for (char &name: mip.name) {
						if (name == '\0') {
							erase = true;
						} else if (erase) {
							name = '\0';
						}
					}
				}
				if (mip.offsets[0] > 0 && mip.width > 0 && mip.height > 0) {
					textures.push_back(mip);
				} else {
					qDebug() << "BAD OFFSET: Texture width: " << mip.width << " height: " << mip.height;
				}
				break;
			case 'E': // Console picture (flat)
				break;
			default:
				qDebug() << "Unknown entry type" << entry.type;
				break;
		}
	}

	return textures;
}

TextureItemModel::TextureCategory textureCategory(const char *name) {
	switch (name[0]) {
		case '+':
			return TextureItemModel::Animated;
		case '{':
			return TextureItemModel::Fence;
		case '*':
			return TextureItemModel::Liquid;
		default:
			if (!qstrncmp(name, "sky", 3))
				return TextureItemModel::Sky;
			return TextureItemModel::Normal;
	}
}

QVariant getCurrentSchemaVersion() {
	QSqlQuery query;
	query.prepare("SELECT version FROM schema_version");
	query.exec();
	query.next();
	return query.value(0);
}

bool shouldApplyMigration(const QVariant& currentSchemaVersion, const QString& fileName) {
	if (currentSchemaVersion.isNull())
		return true;
	const QRegularExpression re("(\\d+)_");
	const QRegularExpressionMatch match = re.match(fileName);
	return !match.hasMatch() || match.captured(1).toInt() > currentSchemaVersion.toInt();
}

bool applyMigrations() {
	QDir migrationDir(":/migrations");

	auto migrationFiles = migrationDir.entryList(QStringList() << "*.sql", QDir::Files);

	std::sort(migrationFiles.begin(), migrationFiles.end());

	const auto currentSchemaVersion = getCurrentSchemaVersion();

	for (const QString &fileName: migrationFiles) {
		if (!shouldApplyMigration(currentSchemaVersion, fileName))
			continue;

		QFile file(migrationDir.filePath(fileName));
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);
			const auto statements = in.readAll().split(';', Qt::SkipEmptyParts);

			for (const QString &statement: statements) {
				QSqlQuery query;
				if (!query.exec(statement.trimmed())) {
					qCritical() << "Migration failed:" << fileName << query.lastError();
					return false;
				}
			}
		} else {
			qCritical() << "Could not open migration file:" << fileName;
			return false;
		}
	}
	return true;
}

QVariant insertWadFile(const QString &filePath, const QDateTime &lastModified, const QString &checksum) {
	QSqlQuery query;
	query.prepare("INSERT INTO wad (path, last_modified, checksum) VALUES (:path, :last_modified, :checksum)");
	query.bindValue(":path", filePath);
	query.bindValue(":last_modified", lastModified.toString(Qt::ISODate));
	query.bindValue(":checksum", checksum);

	if (!query.exec()) {
		qCritical() << "Adding" << filePath << "failed:" << query.lastError().databaseText();
		return QVariant {};
	}
	return QVariant { static_cast<int>(query.lastInsertId().toLongLong()) };
}

#include <QCryptographicHash>

QString calculateSHA1(QFile &file) {
	if (!file.isOpen() && !file.open(QIODevice::ReadOnly)) {
		return QString();  // Or handle the error as needed
	}

	QCryptographicHash hash(QCryptographicHash::Sha1);
	if (hash.addData(&file)) {
		return hash.result().toHex();
	}

	return QString();  // Or handle the error as needed
}

QString calculateSHA1ForRange(QFile &file, qint64 start, qint64 length) {
	if (!file.isOpen() && !file.open(QIODevice::ReadOnly)) {
		qCritical() << "Could not open";
		return QString();
	}

	if (!file.seek(start)) {
		qCritical() << "Could not seek to texture";
		return QString();
	}

	QByteArray data = file.read(length);
	if (data.isEmpty()) {
		qCritical() << "Could not read texture bytes";
		return QString();
	}

	QCryptographicHash hash(QCryptographicHash::Sha1);
	hash.addData(data);

	return hash.result().toHex();
}

QDateTime getLastModified(QFile &file) {
	QFileInfo fileInfo(file);
	return fileInfo.lastModified();
}

bool insertTexture(const dmiptex_t& texture, int wad_id, const QString& checksum) {
	QString textureName = QString::fromLatin1(texture.name, qMin(std::strlen(texture.name), sizeof(texture.name)));

	int offset = texture.offsets[0];

	int category = textureCategory(texture.name);

	QSqlQuery query;
	query.prepare("INSERT INTO texture (wad_id, offset, name, width, height, category, checksum) "
				  "VALUES (:wad_id, :offset, :name, :width, :height, :category, :checksum)");

	query.bindValue(":wad_id", wad_id);
	query.bindValue(":offset", offset);
	query.bindValue(":name", textureName);
	query.bindValue(":width", static_cast<int>(texture.width));
	query.bindValue(":height", static_cast<int>(texture.height));
	query.bindValue(":category", category);
	query.bindValue(":checksum", checksum);

	if (!query.exec()) {
		qDebug() << "Insert texture " << textureName << "failed:" << query.lastError();
		return false;
	}
	return true;
}

bool indexWads() {
	QDir wadDir("/home/daniel/Development/home/quake/maps/wads");

	auto wadFiles = wadDir.entryList(QStringList() << "*.wad", QDir::Files);
	for (const QString &wadFile: wadFiles) {
		QFile file(wadDir.filePath(wadFile));
		if (!file.open(QIODevice::ReadOnly)) {
			qCritical() << "Error: Cannot open file" << wadFile;
			return false;
		}

		QDateTime lastModified = getLastModified(file);

		QSqlQuery query;
		query.prepare("SELECT wad_id, CASE WHEN last_modified < :last_modified THEN checksum ELSE NULL END AS checksum FROM wad WHERE path = :path");
		query.bindValue(":path", file.fileName());
		query.bindValue(":last_modified", lastModified.toString(Qt::ISODate));
		query.exec();
		query.next();

		QVariant wadId;
		QVariant wadChecksum;

		if (query.isValid()) {
			wadId = query.value(0);
			wadChecksum = query.value(1);
		}

		if (wadId.isValid() && wadChecksum.isNull()) {
			// Already know the wad, and last_modified is unchanged
			qDebug() << "skipping known wad";
			continue;
		}

		const QString newChecksum = calculateSHA1(file);
		if (wadChecksum.isValid() && newChecksum == wadChecksum.toString()) {
			// File touched, but no change
			continue;
		}

		wadId = insertWadFile(file.fileName(), lastModified, newChecksum);
		if (wadId.isNull()) {
			// TODO: Something horrible happened
			continue;
		}

		file.seek(0);
		for (const auto &texture : parseWadFile(file)) {
			auto checksum = calculateSHA1ForRange(file, texture.offsets[0], texture.width * texture.height);
			insertTexture(texture, wadId.toInt(), checksum);
		}

		file.close();
	}

	return true;
}

int main(int argc, char *argv[]) {
	QGuiApplication app(argc, argv);

	QString databasePath = "enchilada.db";

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(databasePath);
	if (!db.open()) {
		qCritical() << "Missing SQLite support, exiting.";
		return 1;
	}

	if (!applyMigrations()) {
		return 1;
	}

	if (!indexWads()) {
		return 1;
	}

	qmlRegisterUncreatableMetaObject(
			TextureItemModel::staticMetaObject,
			"enchilada.TextureItemModel",
			1, 0,
			"TextureItemModel",
			"Error: only enums"
	);

	const auto textureItemModel = new TextureItemModel(&app);
	textureItemModel->refresh();

	QQmlApplicationEngine engine;
	engine.addImageProvider(QLatin1String("wad"), new WadImageProvider);
	engine.setInitialProperties({
		{"listViewModel", QVariant::fromValue(textureItemModel)}
	});
	engine.load(QUrl(u"qrc:/qt/qml/enchilada/App.qml"_qs));

	return app.exec();
}
