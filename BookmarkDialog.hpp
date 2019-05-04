#ifndef BOOKMARKDIALOG_HPP
#define BOOKMARKDIALOG_HPP
#include <QWidget>
#include <QDialog>

namespace Ui {
  class BookmarkDialog;
};
class QStringListModel;

class BookmarkDialog : public QDialog {
public:
  BookmarkDialog(const QStringList& bookmarks, QWidget *parent);
  virtual ~BookmarkDialog();
  QStringList getBookmarks() const;

private:
  void connectActions();

  Ui::BookmarkDialog* ui_;
  QStringListModel *model_;
};

#endif
