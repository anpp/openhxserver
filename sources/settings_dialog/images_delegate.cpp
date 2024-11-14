
#include "images_delegate.h"
#include "images_model.h"
#include "../settings.h"


#include <QLineEdit>
#include <QToolButton>
#include <QAction>
#include <QFileDialog>

//-------------------------------------------------------------------------------------------------------------------------------------
QWidget *ImagesDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    row = col = -1;
    if(!index.isValid())
        return nullptr;

    if(index.column() == static_cast<int>(ImagesModel::ImagesModel_defs::FileName))
    {
        QLineEdit *le = new QLineEdit(parent);
        le->setReadOnly(true);

        row = index.row();
        col = index.column();

        QAction *aOpen = le->addAction(QIcon(":/images/icons/folder-open-light.png"), QLineEdit::TrailingPosition);
        aOpen->setToolTip(tr("Open file..."));
        connect(aOpen, &QAction::triggered, this, &ImagesDelegate::openFile);

        if(index.model()->data(index, Qt::DisplayRole).toString() != "")
        {
            QAction *aClear = le->addAction(QIcon(":/images/icons/clear-light.png"), QLineEdit::TrailingPosition);
            aClear->setToolTip(tr("Clear row"));
            connect(aClear, &QAction::triggered, this, &ImagesDelegate::clear);
        }

        return le;
    }
    return nullptr;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ImagesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(!index.isValid()) return;

    if(index.column() == static_cast<int>(ImagesModel::ImagesModel_defs::FileName))
    {
        QLineEdit *le = static_cast<QLineEdit*>(editor);
        if(nullptr != le)
        {
            le->setText(index.model()->data(index, Qt::EditRole).toString());
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ImagesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
    return;
    /*
    if(!index.isValid()) return;

    if(index.column() == static_cast<int>(ImagesModel::ImagesModel_defs::FileName))
    {
        QLineEdit *le = static_cast<QLineEdit*>(editor);
        if(nullptr != le)
        {
            model->setData(index, le->text() , Qt::EditRole);
        }
    }
*/
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ImagesDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid())
        return;

    editor->setGeometry(calcRect(option, index));
}

//-------------------------------------------------------------------------------------------------------------------------------------
QRect ImagesDelegate::calcRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!index.isValid())
        return option.rect;

    if(index.column() == static_cast<int>(ImagesModel::ImagesModel_defs::FileName))
        return option.rect;

    return option.rect;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ImagesDelegate::openFile()
{
    emit closeEditor(static_cast<QLineEdit*>(static_cast<QAction*>(sender())->parent()));

    if(m_model.hasIndex(row, col))
    {
        QString filename;
        QStringList filters;
        QString defaultFilter = tr("Disk images files (*.dsk)");

        filters << defaultFilter << tr("All files (*.*)");

        QFileDialog fd(nullptr, QObject::tr("Open file..."), Settings::instance()->getSetting("directory_dsk").toString(), filters.join(";;"));
        fd.selectNameFilter(defaultFilter);

        connect(&fd, &QFileDialog::filterSelected, this, [&defaultFilter](const QString &filter) {defaultFilter = filter; });
        fd.setFileMode(QFileDialog::ExistingFile);
        fd.setAcceptMode(QFileDialog::AcceptOpen);

        if(fd.exec())
        {
            filename = fd.selectedFiles().at(0);

            m_model.setData(m_model.index(row, col), filename, Qt::EditRole);
        }

    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ImagesDelegate::clear()
{
    if(m_model.hasIndex(row, col))
        m_model.setData(m_model.index(row, col), "", Qt::EditRole);
}
