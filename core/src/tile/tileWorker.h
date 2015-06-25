#pragma once

#include <memory>
#include <future>

#include "util/tileID.h"
#include "data/dataSource.h"
#include "style/style.h"
#include "mapTile.h"
#include "tileTask.h"

class TileWorker {
    
public:
    
    TileWorker();
    
    void processTileData(TileTask _task,
                         const StyleSet& _styles,
                         const View& _view);
    
    void abort();
    
    bool isFinished() const { return m_finished; }

    bool isFree() const { return m_free; }

    // FIXME what if no task is set?
    const TileID& getTileID() const { return m_task->tileID; }
    
    std::shared_ptr<MapTile> getTileResult();
    void drain();
    
private:
    
    bool m_free;
    bool m_aborted;
    bool m_finished;
    
    TileTask m_task;
    //std::future< std::shared_ptr<MapTile> > m_future;
    std::future<bool> m_future;
};

