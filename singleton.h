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
#include <QScopedPointer>
#include "callonce.h"

/**
 * @brief The Singleton class
 */
template <class T>
class Singleton {
    Q_DISABLE_COPY( Singleton )

private:
    typedef T* ( *CreateInstance )();

    Singleton();
    ~Singleton();

    static QBasicAtomicPointer<void> create;
    static QBasicAtomicInt flag;
    static QBasicAtomicPointer<void> singletonPtr;
    static void init();

    bool hasInitialized;

public:
    static T* instance( CreateInstance create );
};

/**
 * @brief Singleton<T>::instance
 * @param create
 * @return
 */
template <class T>
T* Singleton<T>::instance( CreateInstance create ) {
    Singleton::create.store( reinterpret_cast<void*>( create ));
    qCallOnce( Singleton<T>::init, Singleton<T>::flag );
    return (T*)Singleton<T>::singletonPtr.load();
}

/**
 * @brief Singleton<T>::init
 */
template <class T>
void Singleton<T>::init() {
    static Singleton singleton;

    if ( singleton.hasInitialized )
        Singleton<T>::singletonPtr.store((( CreateInstance)Singleton<T>::create.load())());
}

/**
 * @brief Singleton<T>::Singleton
 */
template <class T>
Singleton<T>::Singleton() : hasInitialized( true ) {}

/**
 * @brief Singleton<T>::~Singleton
 */
template <class T>
Singleton<T>::~Singleton() {
    T* createdPtr;

    if ( !this->hasInitialized )
        return;

    createdPtr = (T*)Singleton<T>::singletonPtr.fetchAndStoreOrdered( nullptr );
    if ( createdPtr != nullptr ) {
        this->hasInitialized = false;
        delete createdPtr;
        createdPtr = nullptr;
    }

    Singleton<T>::create.store( nullptr );
}

// define templates
template<class T> QBasicAtomicPointer<void> Singleton<T>::create = Q_BASIC_ATOMIC_INITIALIZER( nullptr );
template<class T> QBasicAtomicInt Singleton<T>::flag = Q_BASIC_ATOMIC_INITIALIZER( Call::Request );
template<class T> QBasicAtomicPointer<void> Singleton<T>::singletonPtr = Q_BASIC_ATOMIC_INITIALIZER( nullptr );

