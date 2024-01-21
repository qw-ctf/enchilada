#ifndef TEXTUREITEMMODEL_H
#define TEXTUREITEMMODEL_H


#include <QSqlQueryModel>

class TextureItemModel : public QSqlQueryModel {
	Q_OBJECT
	Q_PROPERTY(QString searchByName READ searchByName WRITE setSearchByName NOTIFY searchByNameChanged)
	Q_PROPERTY(int searchByType READ searchByType WRITE setSearchByType NOTIFY searchByTypeChanged)

public:
	enum TextureCategory {
		Normal,
		Fence,
		Animated,
		Liquid,
		Sky,
		End,
	};
	Q_ENUM(TextureCategory)

	enum RoleNames {
		TextureIdRole = Qt::UserRole + 1,
		NameRole,
		WidthRole,
		HeightRole,
		CategoryRole
	};

	explicit TextureItemModel(QObject *parent = nullptr) : QSqlQueryModel(parent) {}
	void refresh();

	QString searchByName() const { return m_searchByName; }
	void setSearchByName(const QString &text) {
		if (m_searchByName == text) return;
		m_searchByName = text;
		refresh();
		emit searchByNameChanged();
	}

	int searchByType() const { return m_searchByType; }
	void setSearchByType(const int &value) {
		if (m_searchByType == value) return;
		m_searchByType = value;
		refresh();
		emit searchByTypeChanged();
	}


protected:
	[[nodiscard]] QHash<int, QByteArray> roleNames() const override;
	[[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
	[[nodiscard]] bool canFetchMore(const QModelIndex &parent) const override;
	//void fetchMore(const QModelIndex &parent) override;

signals:
	void searchByNameChanged();
	void searchByTypeChanged();

private:
	QString m_searchByName;
	int m_searchByType = -1;
	int totalRowCount = 0;
};

#endif