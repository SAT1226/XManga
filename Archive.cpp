#include <QDataStream>
#include <QFileInfo>
#include <QDir>

#include <iostream>

#include <unarr.h>
#include <archive.h>
#include <archive_entry.h>

#include "Archive.hpp"

static bool isImageFile(const QString &fileName)
{
  auto ImageFormats = QImageReader::supportedImageFormats();
  foreach(const auto& i, ImageFormats) {
    if(fileName.endsWith("." + i, Qt::CaseInsensitive)) return true;
  }
  return false;
}

class Archive::UnarrLoader : public Archive::Loader {
public:
  UnarrLoader() : open_(false) {}
  virtual ~UnarrLoader() {
    std::cerr << "~UnarrLoader: " << filename_ << std::endl;
    close();
  }

  virtual bool open(const QString &fileName) {
    if(open_) close();

    if(!init(archive_, stream_, fileName)) {
      std::cerr << "unarr:" << std::endl;
      std::cerr << "Could not open file: " << fileName.toLocal8Bit().constData() << std::endl;
      open_ = false;
      return false;
    }

    filename_ = fileName.toLocal8Bit().constData();
    open_ = true;
    return true;
  }

  virtual void close() {
    if(open_) {
      ar_close_archive(archive_);
      ar_close(stream_);
    }
    open_ = false;
  }

  virtual QList<QString> getEntries() {
    QList<QString> entries;
    if(!open_) return entries;

    while (ar_parse_entry(archive_)) {
      QString name = ar_entry_get_name(archive_);

      if(isImageFile(name)) {
        entries.append(name);
      }
    }

    return entries;
  }

  virtual QByteArray getData(const QString &filename) {
    QByteArray data;
    if(!open_) return data;

    ar_parse_entry_for(archive_, filename.toLocal8Bit().constData());
    size_t size = ar_entry_get_size(archive_);

    while (size > 0) {
      unsigned char buffer[1024];
      size_t count = size < sizeof(buffer) ? size : sizeof(buffer);
      if (!ar_entry_uncompress(archive_, buffer, count))
        break;
      size -= count;

      data.append((const char *)buffer, count);
    }

    return data;
  }

  static bool test(const QString &fileName) {
    QFileInfo file(fileName);
    if(!file.isFile()) return false;

    ar_stream *stream;
    ar_archive *archive;
    if(!init(archive, stream, fileName)) return false;

    while (ar_parse_entry(archive)) {
      QString name = ar_entry_get_name(archive);

      if(isImageFile(name)) {
        ar_close_archive(archive);
        ar_close(stream);
        return true;
      }
    }

    return false;
  }

private:
  static ar_archive *ar_open_any_archive(ar_stream *stream, const char *fileext) {
    ar_archive *ar = ar_open_rar_archive(stream);
    if(!ar)
      ar = ar_open_zip_archive(stream, fileext && (strcmp(fileext, ".xps") == 0 || strcmp(fileext, ".epub") == 0));
    if(!ar)
      ar = ar_open_7z_archive(stream);
    if(!ar)
      ar = ar_open_tar_archive(stream);

    return ar;
  }

  static bool init(ar_archive*& archive, ar_stream*& stream, const QString &fileName) {
    stream = ar_open_file(fileName.toLocal8Bit().constData());
    if(!stream) return false;

    archive = ar_open_any_archive(stream, strrchr(fileName.toLocal8Bit().constData(), '.'));
    if(!archive) {
      ar_close(stream);
      return false;
    }

    return true;
  }

  ar_stream *stream_;
  ar_archive *archive_;
  std::string filename_;
  bool open_;
};

class Archive::LibarchiveLoader : public Archive::Loader {
public:
  LibarchiveLoader() : open_(false) {}
  virtual ~LibarchiveLoader() {
    std::cerr << "~LibarchiveLoader: " << filename_.toLocal8Bit().constData() << std::endl;
    close();
  }

  virtual bool open(const QString &fileName) {
    if(open_) close();

    archive *ar;
    if(!init(ar, fileName)) {
      std::cerr << "Libarchive:" << std::endl;
      std::cerr << "Could not open file: " << fileName.toLocal8Bit().constData() << std::endl;
      open_ = false;
      return false;
    }

    filename_ = fileName;
    open_ = true;

    archive_entry *entry;
    while ((archive_read_next_header(ar, &entry)) == ARCHIVE_OK) {
      QString name = archive_entry_pathname(entry);

      if(isImageFile(name)) {
        entries_.append(name);
      }
    }

    archive_read_close(ar);
    archive_read_free(ar);

    return true;
  }

  virtual void close() { entries_.clear(); }
  virtual QList<QString> getEntries() { return entries_; }

  virtual QByteArray getData(const QString &filename) {
    QByteArray data;
    archive *ar;
    archive_entry *entry;
    unsigned char buf[1024];
    int size;

    if(!init(ar, filename_)) return data;
    while ((archive_read_next_header(ar, &entry)) == ARCHIVE_OK) {
      QString name = archive_entry_pathname(entry);

      if (name == filename) {
        while ((size = archive_read_data(ar, buf, sizeof(buf))) > 0) {
          data.append((const char *)buf, size);
        }
        break;
      }
    }
    archive_read_close(ar);
    archive_read_free(ar);

    return data;
  }

  static bool test(const QString &fileName) {
    QFileInfo file(fileName);
    if(!file.isFile()) return false;

    archive *ar;
    if(!init(ar, fileName)) return false;

    archive_entry *entry;
    while ((archive_read_next_header(ar, &entry)) == ARCHIVE_OK) {
      QString name = archive_entry_pathname(entry);

      if(isImageFile(name)) {
        archive_read_close(ar);
        archive_read_free(ar);
        return true;
      }
    }

    archive_read_close(ar);
    archive_read_free(ar);
    return false;
  }

private:
  static bool init(archive*& ar, const QString &fileName) {
    ar = archive_read_new();
    archive_read_support_format_rar5(ar);
    archive_read_support_filter_all(ar);

    if(archive_read_open_filename(ar, fileName.toLocal8Bit().constData(),
                                   10240) != ARCHIVE_OK) {
      archive_read_free(ar);
      return false;
    }

    return true;
  }

  QList<QString> entries_;
  QString filename_;
  bool open_;
};

class Archive::DirectoryLoader : public Archive::Loader {
public:
  DirectoryLoader() : open_(false) {}
  virtual ~DirectoryLoader() {
    std::cerr << "~DirectoryLoader: " << directoryPath_.toLocal8Bit().constData() << std::endl;
  }

  virtual bool open(const QString &fileName) {
    QFileInfo file(fileName);
    if(!file.isDir()) return false;

    directoryPath_ = fileName;
    open_ = true;
    return true;
  }

  virtual void close() {
    open_ = false;
  }

  virtual QList<QString> getEntries() {
    QList<QString> entries;
    if(!open_) return entries;

    QDir dir = QDir(directoryPath_);
    entries = dir.entryList(getImageExts(), QDir::Files);

    QCollator sorter;
    sorter.setNumericMode(true);
    sorter.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(entries.begin(), entries.end(), sorter);

    return entries;
  }

  virtual QByteArray getData(const QString &filename) {
    QByteArray data;
    if(!open_) return data;

    QFileInfo f(directoryPath_, filename);
    QFile file(f.absoluteFilePath());
    if(!file.open(QIODevice::ReadOnly)) {
      return data;
    }

    data = file.readAll();
    return data;
  }

  static bool test(const QString &fileName) {
    QFileInfo file(fileName);
    if(!file.isDir()) return false;

    QList<QString> entries;
    QDir dir = QDir(fileName);
    entries = dir.entryList(getImageExts(), QDir::Files);

    if(entries.empty()) return false;
    return true;
  }


private:
  static QStringList getImageExts() {
    QStringList filters;
    auto imageFormats = QImageReader::supportedImageFormats();
    foreach(const auto& i, imageFormats) {
      filters << "*." + i;
    }

    return filters;
  }

  bool open_;
  QString directoryPath_;
};

bool Archive::open(const QString &fileName)
{
  if(open_) entryNameList_.clear();;

  QFileInfo file(fileName);
  if(file.isDir()) {
    loader_.reset(new Archive::DirectoryLoader());

    if(!loader_->open(fileName)) {
      open_ = false;
      return false;
    }
  }
  else {
    loader_.reset(new Archive::UnarrLoader());
    if (!loader_->open(fileName)) {
      loader_.reset(new Archive::LibarchiveLoader());
      if (!loader_->open(fileName)) {
        open_ = false;
        return false;
      }
    }
  }
  entryNameList_ = loader_ -> getEntries();

  QCollator sorter;
  sorter.setNumericMode(true);
  sorter.setCaseSensitivity(Qt::CaseInsensitive);

  std::sort(entryNameList_.begin(), entryNameList_.end(), sorter);

  open_ = true;
  return true;
}

bool Archive::test(const QString &fileName)
{
  if (!DirectoryLoader::test(fileName)) {
    if (!UnarrLoader::test(fileName)) {
      if (!LibarchiveLoader::test(fileName)) {
        return false;
      }
    }
  }

  return true;
}

QByteArray Archive::getData(const QString &filename)
{
  return loader_ -> getData(filename);
}
