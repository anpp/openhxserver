
#include "images_model.h"
#include "../server/images.h"
#include "../server/imagedsk.h"

//-----------------------------------------------------------------------------------------------------------------
QVariant ImagesModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid())
    {
        if(Qt::EditRole == role || Qt::DisplayRole == role)
            return value(index.row(), index.column(), role);
    }
    return QVariant();
}

//-----------------------------------------------------------------------------------------------------------------
QVariant ImagesModel::value(int row, int col, int role) const
{
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
bool ImagesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;

    QString filename = "";
    if(Qt::EditRole == role)
        switch(index.column())
        {
        case static_cast<int>(ImagesModel_defs::FileName):
            filename = value.toString();
            m_data.data().at(index.row())->setFileName(value.toString());
            emit selected_file(value.toString());

            emit dataChanged(QModelIndex(), QModelIndex());
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

