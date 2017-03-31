/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DFILESYSTEMWATCHER_P_H
#define DFILESYSTEMWATCHER_P_H

#include "dfilesystemwatcher.h"
#include "private/dobject_p.h"

#include <QSocketNotifier>
#include <QHash>
#include <QMap>

DUTIL_BEGIN_NAMESPACE

class DFileSystemWatcherPrivate : public DObjectPrivate
{
    Q_DECLARE_PUBLIC(DFileSystemWatcher)

public:
    DFileSystemWatcherPrivate(int fd, DFileSystemWatcher *qq);
    ~DFileSystemWatcherPrivate();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    QStringList files, directories;
    int inotifyFd;
    QHash<QString, int> pathToID;
    QMultiHash<int, QString> idToPath;
    QSocketNotifier notifier;

    // private slots
    void _q_readFromInotify();

private:
    QString getPathFromID(int id) const;
    void onFileChanged(const QString &path, bool removed);
    void onDirectoryChanged(const QString &path, bool removed);
};

DUTIL_END_NAMESPACE

#endif // DFILESYSTEMWATCHER_P_H
