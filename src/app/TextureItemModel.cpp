#include <QSqlQuery>
#include "TextureItemModel.h"

QHash<int, QByteArray> TextureItemModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[TextureIdRole] = "texture_id";
	roles[NameRole] = "name";
	roles[WidthRole] = "width";
	roles[HeightRole] = "height";
	roles[CategoryRole] = "category";
	return roles;
}

QVariant TextureItemModel::data(const QModelIndex &index, int role) const {
	if (role < Qt::UserRole) {
		return QSqlQueryModel::data(index, role);
	}
	int columnIdx = role - Qt::UserRole - 1;
	QModelIndex modelIndex = createIndex(index.row(), columnIdx);
	return QSqlQueryModel::data(modelIndex, Qt::DisplayRole);
}

void TextureItemModel::refresh() {
	QStringList conditions;
	QString where;
	conditions.append("category != 2");
	if (!m_searchByName.isEmpty()) {
		conditions.append("name LIKE :name");
	}
	if (m_searchByType != -1) {
		conditions.append("category = :category");
	}
	if (!conditions.isEmpty()) {
		where = " WHERE " + conditions.join(" AND ");
	}

	QSqlQuery countQuery;
	countQuery.prepare("SELECT COUNT(*) FROM texture" + where);

	if (!m_searchByName.isEmpty()) {
		countQuery.bindValue(":name", "%" + m_searchByName + "%");
	}
	if (m_searchByType != -1) {
		countQuery.bindValue(":category", m_searchByType);
	}

	countQuery.exec();
	countQuery.next();
	totalRowCount = countQuery.value(0).toInt();

	QSqlQuery query;

	query.prepare("SELECT texture_id, name, width, height, category FROM texture" + where + " ORDER BY wad_id, name");

	if (!m_searchByName.isEmpty()) {
		query.bindValue(":name", "%" + m_searchByName + "%");
	}
	if (m_searchByType != -1) {
		query.bindValue(":category", m_searchByType);
	}

	query.exec();

	setQuery(std::move(query));
}

bool TextureItemModel::canFetchMore(const QModelIndex &parent) const {
	if (parent.isValid()) return false;
	return rowCount(parent) < totalRowCount;
}