#include "FilterProxyModel.h"

#include "TextureItemModel.h"

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    if (m_searchByType >= TextureItemModel::Normal && m_searchByType < TextureItemModel::End) {
        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        if (sourceModel()->data(index, TextureItemModel::RoleNames::ImageTypeRole) != m_searchByType) {
            return false;
        }
    }
    if (!m_searchByName.isEmpty()) {
        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        return sourceModel()->data(index).toString().contains(m_searchByName, Qt::CaseInsensitive);
    }
    return true;
}
