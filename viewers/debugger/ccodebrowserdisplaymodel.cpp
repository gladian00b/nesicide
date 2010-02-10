#include "ccodebrowserdisplaymodel.h"

#include "cnesrom.h"

CCodeBrowserDisplayModel::CCodeBrowserDisplayModel(QObject* parent)
{
}

CCodeBrowserDisplayModel::~CCodeBrowserDisplayModel()
{
}

QVariant CCodeBrowserDisplayModel::data(const QModelIndex &index, int role) const
{
   if (!index.isValid())
      return QVariant();

   if (role != Qt::DisplayRole)
      return QVariant();

   return (char*)index.internalPointer();
}

Qt::ItemFlags CCodeBrowserDisplayModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return flags;
}

QVariant CCodeBrowserDisplayModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   char buffer [ 6 ];

   if (role != Qt::DisplayRole)
      return QVariant();

   if ( orientation == Qt::Horizontal )
   {
      return "Disassembly";
   }
   else
   {
      int sloc8000 = CROM::SLOC(0x8000);
      int slocA000 = CROM::SLOC(0xA000);
      int slocC000 = CROM::SLOC(0xC000);
      int slocE000 = CROM::SLOC(0xE000);
      int addr;

      if ( section < sloc8000 )
      {
         addr = 0x8000+CROM::SLOC2ADDR(0x8000,section);
      }
      else if ( section < (sloc8000+slocA000) )
      {
         addr = 0xA000+CROM::SLOC2ADDR(0xA000,section-sloc8000);
      }
      else if ( section < (sloc8000+slocA000+slocC000) )
      {
         addr = 0xC000+CROM::SLOC2ADDR(0xC000,section-sloc8000-slocA000);
      }
      else if ( section < (sloc8000+slocA000+slocC000+slocE000) )
      {
         addr = 0xE000+CROM::SLOC2ADDR(0xE000,section-sloc8000-slocA000-slocC000);
      }

      sprintf ( buffer, "$%04X", addr );
      return buffer;
   }

   return  QVariant();
}

QModelIndex CCodeBrowserDisplayModel::index(int row, int column, const QModelIndex &parent) const
{
   int sloc8000 = CROM::SLOC(0x8000);
   int slocA000 = CROM::SLOC(0xA000);
   int slocC000 = CROM::SLOC(0xC000);
   int slocE000 = CROM::SLOC(0xE000);
   int addr;

   if ( row < sloc8000 )
   {
      addr = 0x8000+CROM::SLOC2ADDR(0x8000,row);
   }
   else if ( row < (sloc8000+slocA000) )
   {
      addr = 0xA000+CROM::SLOC2ADDR(0xA000,row-sloc8000);
   }
   else if ( row < (sloc8000+slocA000+slocC000) )
   {
      addr = 0xC000+CROM::SLOC2ADDR(0xC000,row-sloc8000-slocA000);
   }
   else if ( row < (sloc8000+slocA000+slocC000+slocE000) )
   {
      addr = 0xE000+CROM::SLOC2ADDR(0xE000,row-sloc8000-slocA000-slocC000);
   }

   return createIndex(row, column, CROM::DISASSEMBLY(addr));
}

int CCodeBrowserDisplayModel::rowCount(const QModelIndex &parent) const
{
   // Get the source-lines-of-code count from each disassembled ROM
   // bank that is currently visible to the CPU...
   int rows = CROM::SLOC(0x8000)+CROM::SLOC(0xA000)+CROM::SLOC(0xC000)+CROM::SLOC(0xE000);

   return rows;
}

int CCodeBrowserDisplayModel::columnCount(const QModelIndex &parent) const
{
   if ( parent.isValid())
   {
      return 0;
   }
   return 1;
}

void CCodeBrowserDisplayModel::layoutChangedEvent()
{
    this->layoutChanged();
}