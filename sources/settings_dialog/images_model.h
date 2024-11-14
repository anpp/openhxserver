
#ifndef IMAGESMODEL_H
#define IMAGESMODEL_H

#include <QAbstractItemModel>

class Images;

class ImagesModel : public QAbstractItemModel
{
    Q_OBJECT    

    Images& m_data;
public:
    enum class ImagesModel_defs: int {NumColumns = 3, HXIndex = 0, ImageName = 1, FileName = 2};

    explicit ImagesModel(Images& data, QObject *parent = nullptr):
        QAbstractItemModel(parent), m_data(data) {};

    QModelIndex index(int row, int column, const QModelIndex & = QModelIndex()) const override { return createIndex(row, column);}
    QModelIndex parent(const QModelIndex &) const override { return QModelIndex(); }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override {return static_cast<int>(ImagesModel_defs::NumColumns); }
    QVariant value(int row, int col, int role = Qt::DisplayRole) const;

    void save() const;

signals:
    void selected_file(const QString&);
};

#endif // IMAGESMODEL_H
