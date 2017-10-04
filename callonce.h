/*
 * Copyright (C) 2017 Zvaigznu Planetarijs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

#pragma once

//
// includes
//
#include <QtGlobal>
#include <QAtomicInt>
#include <QMutex>
#include <QWaitCondition>
#include <QThreadStorage>
#include <QThread>

/**
 * The Call namespace
 */
namespace Call {
    enum Type {
        Request,
        InProgress,
        Finished
    };
}

/**
 * @brief qCallOnce
 * @param func
 * @param flag
 */
template <class Function>
inline static void qCallOnce( Function func, QBasicAtomicInt &flag ) {
    using namespace Call;
    int protectFlag;

    protectFlag = flag.fetchAndStoreAcquire( flag.load());

    if ( protectFlag == Finished )
        return;

    if ( protectFlag == Request && flag.testAndSetRelaxed( protectFlag, InProgress )) {
        func();
        flag.fetchAndStoreRelease( Finished );
    } else {
        do QThread::yieldCurrentThread();
        while ( !flag.testAndSetAcquire( Finished, Finished ));
    }
}
