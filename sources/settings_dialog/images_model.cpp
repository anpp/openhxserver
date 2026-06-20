
#include "images_model.h"
#include "../server/images.h"
#include "../server/imagedsk.h"

#include <QFont>
#include <QDir>

//-----------------------------------------------------------------------------------------------------------------
QVariant ImagesModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid())
    {
        if(Qt::EditRole == role || Qt::DisplayRole == role || role > Qt::UserRole)
            return value(index.row(), index.column(), role);

        if(role == Qt::FontRole)
            switch(index.column())
            {
            case static_cast<int>(ImagesModel_defs::ImageName):
                if(m_data.at(index.row()).needReload())
                {
                    QFont font;
                    font.setBold(true);
                    return font;
                }
                return QVariant();
            default:
                return QVariant();
            }

    }
    return QVariant();
}

//-----------------------------------------------------------------------------------------------------------------
QVariant ImagesModel::value(int row, int col, int role) const
{
    if (role > Qt::UserRole)
    {
        switch (role)
        {
        case HXIndexRole:
            return m_data.prefix() + QString::number(row);
        case ImageNameRole:
            return m_data.at(row).shortFileName();
        case FileNameRole:
            return m_data.at(row).fileName();
        default:
            return QVariant();
        }
    }

    if(Qt::EditRole == role || Qt::DisplayRole == role)
        switch(col)
        {
        case static_cast<int>(ImagesModel_defs::HXIndex):
            return m_data.prefix() + QString::number(row);
        case static_cast<int>(ImagesModel_defs::ImageName):
            return m_data.at(row).shortFileName();
        case static_cast<int>(ImagesModel_defs::FileName):
            return m_data.at(row).fileName();

        default:
            return QVariant();
        }
    return QVariant();
}

//-----------------------------------------------------------------------------------------------------------------
void ImagesModel::save() const
{
    m_data.save();
}

//-----------------------------------------------------------------------------------------------------------------
void ImagesModel::setFileNameAt(int row, const QString &filePath)
{
    QModelIndex idx = this->index(row, 0);
    this->setData(idx, filePath, FileNameRole);
}

//-----------------------------------------------------------------------------------------------------------------
QString ImagesModel::getFileNameAt(int row)
{
    return value(row, 0, FileNameRole).toString();
}

//-----------------------------------------------------------------------------------------------------------------
bool ImagesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;

    QString filename = "";

    if (role > Qt::UserRole)
    {
        switch (role)
        {
        case FileNameRole:
            filename = QDir::toNativeSeparators(value.toString());

            m_data.data().at(index.row())->setFileName(value.toString());
            emit selected_file(value.toString());

            emit dataChanged(index, index);
            return true;

        default:
            break;
        }
    }

    if(Qt::EditRole == role)
        switch(index.column())
        {
        case static_cast<int>(ImagesModel_defs::FileName):
            filename = value.toString();
            m_data.data().at(index.row())->setFileName(value.toString());
            emit selected_file(value.toString());

            emit dataChanged(index, index);
            return true;
        default:
            break;
        }

    return false;
}

//-----------------------------------------------------------------------------------------------------------------
QVariant ImagesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Vertical)
        return section + 1;

    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
        switch(section)
        {
        case static_cast<int>(ImagesModel_defs::HXIndex):
            return tr("Device");
        case static_cast<int>(ImagesModel_defs::ImageName):
            return tr("Image");
        case static_cast<int>(ImagesModel_defs::FileName):
            return tr("File");
        default:
            return QVariant();
        }
    return QVariant();
}

//-----------------------------------------------------------------------------------------------------------------
QHash<int, QByteArray> ImagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[HXIndexRole] = "hxIndex";
    roles[ImageNameRole] = "imageName";
    roles[FileNameRole] = "fileName";
    return roles;
}

//-----------------------------------------------------------------------------------------------------------------
Qt::ItemFlags ImagesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    flags &= ~Qt::ItemIsEditable;
    if(index.isValid())
    {
        if(index.column() == static_cast<int>(ImagesModel_defs::FileName))
            flags |= Qt::ItemIsEditable;
    }
    return flags;
}

//-----------------------------------------------------------------------------------------------------------------
int ImagesModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(m_data.size());
}

