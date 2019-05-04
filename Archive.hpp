#ifndef Archive_HPP
#define Archive_HPP
#include <QString>
#include <QCollator>

#include <iostream>
#include <unarr.h>

class Archive {
public:
  Archive() {
    stream_ = 0;
    archive_ = 0;
    open_ = false;
  }

  virtual ~Archive() {}

  void Close() {
    ar_close_archive(archive_);
    ar_close(stream_);
    open_ = false;
  }

  bool Open(const QString& fileName) {
    if(open_) {
      Close();
      entryNameList_.clear();
    }

    stream_ = ar_open_file(fileName.toLocal8Bit().constData());
    if(!stream_) {
      std::cerr << "Could not open file: " << fileName.toLocal8Bit().constData() << std::endl;

      open_ = false;
      return false;
    }

    archive_ = ar_open_any_archive(stream_, strrchr(fileName.toLocal8Bit().constData(), '.'));
    if(!archive_) {
      std::cerr << "Could not open file: " << fileName.toLocal8Bit().constData() << std::endl;
      ar_close(stream_);
      open_ = false;
      return false;
    }
    while (ar_parse_entry(archive_)) {
        QString name = ar_entry_get_name(archive_);

        if(name.endsWith(".jpg", Qt::CaseInsensitive) ||
           name.endsWith(".jpeg", Qt::CaseInsensitive) ||
           name.endsWith(".png", Qt::CaseInsensitive) ||
           name.endsWith(".bmp", Qt::CaseInsensitive)) {
          entryNameList_.append(name);
        }
    }

    QCollator sorter;
    sorter.setNumericMode(true);
    sorter.setCaseSensitivity(Qt::CaseInsensitive);

    std::sort(entryNameList_.begin(), entryNameList_.end(), sorter);

    open_ = true;
    return true;
  }

  QList<QString> getPathNameList() {
    return entryNameList_;
  }

  QByteArray getData(const QString& filename) {
    QByteArray data;

    ar_parse_entry_for(archive_, filename.toLocal8Bit().constData());
    size_t size = ar_entry_get_size(archive_);

    while (size > 0) {
      unsigned char buffer[1024];
      size_t count = size < sizeof(buffer) ? size : sizeof(buffer);
      if (!ar_entry_uncompress(archive_, buffer, count))
        break;
      size -= count;

      data.append((const char*)buffer, count);
    }

    return data;
  }

private:
  ar_archive *ar_open_any_archive(ar_stream *stream, const char *fileext) {
    ar_archive *ar = ar_open_rar_archive(stream);
    if(!ar)
      ar = ar_open_zip_archive(stream, fileext && (strcmp(fileext, ".xps") == 0 || strcmp(fileext, ".epub") == 0));
    if(!ar)
      ar = ar_open_7z_archive(stream);
    if(!ar)
      ar = ar_open_tar_archive(stream);

    return ar;
  }

  ar_stream *stream_;
  ar_archive *archive_;
  bool open_;
  QList<QString> entryNameList_;
};

#endif
