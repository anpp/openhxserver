
#ifndef IMAGES_DELEGATE_H
#define IMAGES_DELEGATE_H

#include <QItemDelegate>

class QAbstractItemModel;

class ImagesDelegate : public QItemDelegate
{
    QAbstractItemModel& m_model;
public:
    explicit ImagesDelegate(QAbstractItemModel& model, QObject *parent = nullptr) : QItemDelegate(parent), m_model(model) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QRect calcRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    mutable int row = -1;
    mutable int col = -1;


private slots:
    void openFile();
    void clear();
};

#endif // IMAGES_DELEGATE_H
