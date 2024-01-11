#ifndef FILTERPROXYMODEL_H
#define FILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class FilterProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString searchByName READ searchByName WRITE setSearchByName NOTIFY searchByNameChanged)
    Q_PROPERTY(int searchByType READ searchByType WRITE setSearchByType NOTIFY searchByTypeChanged)

public:
    explicit FilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent), m_searchByType(-1) {}

    QString searchByName() const { return m_searchByName; }
    void setSearchByName(const QString &text) {
        if (m_searchByName == text) return;
        m_searchByName = text;
        invalidateFilter();
        emit searchByNameChanged();
    }

    int searchByType() const { return m_searchByType; }
    void setSearchByType(const int &value) {
        if (m_searchByType == value) return;
        m_searchByType = value;
        invalidateFilter();
        emit searchByTypeChanged();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

signals:
    void searchByNameChanged();
    void searchByTypeChanged();

private:
    QString m_searchByName;
    int m_searchByType;
};

#endif //FILTERPROXYMODEL_H
