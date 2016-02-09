/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009, 2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Map Tile Cache Data
 *
 *   @author Gus Grubba <mavlink@grubba.com>
 *
 */

#ifndef QGC_MAP_ENGINE_DATA_H
#define QGC_MAP_ENGINE_DATA_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QDateTime>

#include "QGCMapUrlEngine.h"

class QGCCachedTileSet;

//-----------------------------------------------------------------------------
class QGCTile : public QObject
{
    Q_OBJECT
public:
    QGCTile()
        : _x(0)
        , _y(0)
        , _z(0)
        , _set(UINT64_MAX)
        , _type(UrlFactory::Invalid)
    {
    }

    QGCTile(const QGCTile& other)
        : _x(other.x())
        , _y(other.y())
        , _z(other.z())
        , _set(other.set())
        , _hash(other.hash())
        , _type(other.type())
    {
    }

    enum TyleState {
        StatePending = 0,
        StateDownloading,
        StateError,
        StateComplete
    };

    int                 x           () const { return _x; }
    int                 y           () const { return _y; }
    int                 z           () const { return _z; }
    qulonglong          set         () const { return _set;  }
    const QString       hash        () const { return _hash; }
    UrlFactory::MapType type        () const { return _type; }

    void                setX        (int x) { _x = x; }
    void                setY        (int y) { _y = y; }
    void                setZ        (int z) { _z = z; }
    void                setTileSet  (qulonglong set) { _set = set;  }
    void                setHash     (const QString& hash) { _hash = hash; }
    void                setType     (UrlFactory::MapType type) { _type = type; }

private:
    int         _x;
    int         _y;
    int         _z;
    qulonglong  _set;
    QString     _hash;
    UrlFactory::MapType _type;
};

//-----------------------------------------------------------------------------
class QGCCacheTile : public QObject
{
    Q_OBJECT
public:
    QGCCacheTile    (const QString hash, const QByteArray img, const QString format, UrlFactory::MapType type, qulonglong set = UINT64_MAX)
        : _set(set)
        , _hash(hash)
        , _img(img)
        , _format(format)
        , _type(type)
    {
    }
    QGCCacheTile    (const QString hash, qulonglong set)
        : _set(set)
        , _hash(hash)
    {
    }
    qulonglong          set     () { return _set;   }
    QString             hash    () { return _hash;  }
    QByteArray          img     () { return _img;   }
    QString             format  () { return _format;}
    UrlFactory::MapType type    () { return _type; }
private:
    qulonglong  _set;
    QString     _hash;
    QByteArray  _img;
    QString     _format;
    UrlFactory::MapType _type;
};

//-----------------------------------------------------------------------------
class QGCMapTask : public QObject
{
    Q_OBJECT
public:

    enum TaskType {
        taskInit,
        taskCacheTile,
        taskFetchTile,
        taskFetchTileSets,
        taskCreateTileSet,
        taskGetTileDownloadList,
        taskUpdateTileDownloadState,
        taskDeleteTileSet,
        taskPruneCache,
        taskReset
    };

    QGCMapTask(TaskType type)
        : _type(type)
    {}
    virtual ~QGCMapTask()
    {}

    virtual TaskType    type            () { return _type; }

    void setError(QString errorString)
    {
        emit error(_type, errorString);
    }

signals:
    void error          (QGCMapTask::TaskType type, QString errorString);

private:
    TaskType    _type;
};

//-----------------------------------------------------------------------------
class QGCFetchTileSetTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCFetchTileSetTask()
        : QGCMapTask(QGCMapTask::taskFetchTileSets)
    {}

    void setTileSetFetched(QGCCachedTileSet* tileSet)
    {
        emit tileSetFetched(tileSet);
    }

signals:
    void            tileSetFetched  (QGCCachedTileSet* tileSet);
};

//-----------------------------------------------------------------------------
class QGCCreateTileSetTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCCreateTileSetTask(QGCCachedTileSet* tileSet)
        : QGCMapTask(QGCMapTask::taskCreateTileSet)
        , _tileSet(tileSet)
        , _saved(false)
    {}

    ~QGCCreateTileSetTask();

    QGCCachedTileSet*   tileSet () { return _tileSet; }

    void setTileSetSaved()
    {
        //-- Flag as saved. Signalee wll maintain it.
        _saved = true;
        emit tileSetSaved(_tileSet);
    }

signals:
    void tileSetSaved   (QGCCachedTileSet* tileSet);

private:
    QGCCachedTileSet* _tileSet;
    bool              _saved;
};

//-----------------------------------------------------------------------------
class QGCFetchTileTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCFetchTileTask(const QString hash)
        : QGCMapTask(QGCMapTask::taskFetchTile)
        , _hash(hash)
    {}

    ~QGCFetchTileTask()
    {
    }

    void setTileFetched(QGCCacheTile* tile)
    {
        emit tileFetched(tile);
    }

    QString         hash() { return _hash; }

signals:
    void            tileFetched     (QGCCacheTile* tile);

private:
    QString         _hash;
};

//-----------------------------------------------------------------------------
class QGCSaveTileTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCSaveTileTask(QGCCacheTile* tile)
        : QGCMapTask(QGCMapTask::taskCacheTile)
        , _tile(tile)
    {}

    ~QGCSaveTileTask()
    {
        if(_tile)
            delete _tile;
    }

    QGCCacheTile*   tile() { return _tile; }

private:
    QGCCacheTile*   _tile;
};

//-----------------------------------------------------------------------------
class QGCGetTileDownloadListTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCGetTileDownloadListTask(qulonglong setID, int count)
        : QGCMapTask(QGCMapTask::taskGetTileDownloadList)
        , _setID(setID)
        , _count(count)
    {}

    qulonglong  setID() { return _setID; }
    int         count() { return _count; }

    void setTileListFetched(QList<QGCTile*> tiles)
    {
        emit tileListFetched(tiles);
    }

signals:
    void            tileListFetched  (QList<QGCTile*> tiles);

private:
    qulonglong  _setID;
    int         _count;
};

//-----------------------------------------------------------------------------
class QGCUpdateTileDownloadStateTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCUpdateTileDownloadStateTask(qulonglong setID, QGCTile::TyleState state, const QString& hash)
        : QGCMapTask(QGCMapTask::taskUpdateTileDownloadState)
        , _setID(setID)
        , _state(state)
        , _hash(hash)
    {}

    QString             hash    () { return _hash; }
    qulonglong          setID   () { return _setID; }
    QGCTile::TyleState  state   () { return _state; }

private:
    qulonglong          _setID;
    QGCTile::TyleState  _state;
    QString             _hash;
};

//-----------------------------------------------------------------------------
class QGCDeleteTileSetTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCDeleteTileSetTask(qulonglong setID)
        : QGCMapTask(QGCMapTask::taskDeleteTileSet)
        , _setID(setID)
    {}

    qulonglong  setID() { return _setID; }

    void setTileSetDeleted()
    {
        emit tileSetDeleted(_setID);
    }

signals:
    void tileSetDeleted(qulonglong setID);

private:
    qulonglong  _setID;
};

//-----------------------------------------------------------------------------
class QGCPruneCacheTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCPruneCacheTask(quint64 amount)
        : QGCMapTask(QGCMapTask::taskPruneCache)
        , _amount(amount)
    {}

    quint64  amount() { return _amount; }

    void setPruned()
    {
        emit pruned();
    }

signals:
    void pruned();

private:
    quint64  _amount;
};

//-----------------------------------------------------------------------------
class QGCResetTask : public QGCMapTask
{
    Q_OBJECT
public:
    QGCResetTask()
        : QGCMapTask(QGCMapTask::taskReset)
    {}

    void setResetCompleted()
    {
        emit resetCompleted();
    }

signals:
    void resetCompleted();
};


#endif // QGC_MAP_ENGINE_DATA_H
