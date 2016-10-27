/*===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#include "path-priv.h" /* VPathGetScheme_t */
#include "services-priv.h"

#include <klib/rc.h> /* RC */
#include <klib/vector.h> /* Vector */
#include <vfs/path.h> /* VPath */
#include <atomic.h> /* atomic32_dec_and_test */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

struct VPathSet {
    atomic32_t refcount; 
    const VPath * fasp;
    const VPath * file;
    const VPath * http;
    const VPath * https;
    const VPath * cacheFasp;
    const VPath * cacheFile;
    const VPath * cacheHttp;
    const VPath * cacheHttps;
};

struct VPathSetList {
    atomic32_t refcount;
    Vector list;
};

/* VPathSet */
rc_t VPathSetAddRef ( const VPathSet * self ) {
    if ( self != NULL ) {
        atomic32_inc ( & ( ( VPathSet * ) self ) -> refcount );
    }
    return 0;
}

rc_t VPathSetWhack ( VPathSet * self ) {
    rc_t rc = 0;

    if ( self != NULL ) {
        RELEASE ( VPath, self -> fasp );
        RELEASE ( VPath, self -> file );
        RELEASE ( VPath, self -> http );
        RELEASE ( VPath, self -> https );
        RELEASE ( VPath, self -> cacheFasp );
        RELEASE ( VPath, self -> cacheFile );
        RELEASE ( VPath, self -> cacheHttp );
        RELEASE ( VPath, self -> cacheHttps );

        free ( self );
    }

    return rc;
}

static void whackVPathSet  ( void * self, void * ignore ) {
    VPathSetWhack ( ( VPathSet * ) self);
}

rc_t VPathSetRelease ( const VPathSet * cself ) {
    VPathSet * self = ( VPathSet * ) cself;

    if ( self != NULL && atomic32_dec_and_test ( & self -> refcount ) ) {
        return VPathSetWhack ( self );
    }

    return 0;
}

rc_t VPathSetGet ( const VPathSet * self, VRemoteProtocols protocols,
    const VPath ** path, const VPath ** vdbcache )
{
    rc_t rc = 0;
    VRemoteProtocols protocol = protocols;
    const VPath * p = NULL;
    const VPath * c = NULL;
    if ( self == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    }
    for ( ; protocol != 0; protocol >>= 3 ) {
        switch ( protocol & eProtocolMask ) {
            case eProtocolFasp:
                p = self -> fasp;
                c = self -> cacheFasp;
                break;
            case eProtocolFile:
                p = self -> file;
                c = self -> cacheFile;
                break;
            case eProtocolHttp:
                p = self -> http;
                c = self -> cacheHttp;
                break;
            case eProtocolHttps:
                p = self -> https;
                c = self -> cacheHttps;
                break;
            default:
                assert ( 0 );
                return -1;
        }
        if ( p != NULL || c != NULL ) {
            if ( path != NULL ) {
                rc = VPathAddRef ( p );
                if ( rc == 0 ) {
                    * path = p;
                }
            }
            if ( vdbcache != NULL ) {
                rc_t r2 = VPathAddRef ( c );
                if ( r2 == 0 ) {
                    * vdbcache = c;
                } else if ( rc == 0) {
                    rc = r2;
                }
            }
            return rc;
        }
    }
    return 0;
}

rc_t VPathSetMake
    ( VPathSet ** self, const EVPath * src, bool singleUrl )
{
    VPathSet * p = NULL;
    rc_t rc = 0;
    rc_t r2 = 0;
    assert ( self && src );
    p = ( VPathSet * ) calloc ( 1, sizeof * p );
    if ( p == NULL ) {
        return RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );
    }
    if ( singleUrl ) {
        VPUri_t uri_type = vpuri_invalid;
        rc = VPathGetScheme_t ( src -> http, & uri_type );
        if ( rc == 0 ) {
            const VPath ** d = NULL;
            switch ( uri_type ) {
                case vpuri_fasp:
                    d = & p -> fasp;
                    break;
                case vpuri_file:
                    d = & p -> file;
                    break;
                case vpuri_http:
                    d = & p -> http;
                    break;
                case vpuri_https:
                    d = & p -> https;
                    break;
                default:
                    assert ( 0 );
                    return -1;
            }
            r2 = VPathAddRef ( src -> http );
            if ( r2 == 0 ) {
                * d = src -> http;
            } else if ( rc == 0 ) {
                rc = r2;
            }
        }
    }
    else {
        r2 = VPathAddRef ( src -> fasp );
        if ( r2 == 0 ) {
            p -> fasp = src -> fasp;
        } else if ( rc == 0 ) {
            rc = r2;
        }
        r2 = VPathAddRef ( src -> file );
        if ( r2 == 0 ) {
            p -> file = src -> file;
        } else if ( rc == 0 ) {
            rc = r2;
        }
        r2 = VPathAddRef ( src -> http );
        if ( r2 == 0 ) {
            p -> http = src -> http;
        } else if ( rc == 0 ) {
            rc = r2;
        }
        r2 = VPathAddRef ( src -> https );
        if ( r2 == 0 ) {
            p -> https = src -> https;
        } else if ( rc == 0 ) {
            rc = r2;
        }
        r2 = VPathAddRef ( src -> vcFasp );
        if ( r2 == 0 ) {
            p -> cacheFasp = src -> vcFasp;
        } else if ( rc == 0 ) {
            rc = r2;
        }
        r2 = VPathAddRef ( src -> vcFile );
        if ( r2 == 0 ) {
            p -> cacheFile = src -> vcFile;
        } else if ( rc == 0 ) {
            rc = r2;
        }
        r2 = VPathAddRef ( src -> vcHttp );
        if ( r2 == 0 ) {
            p -> cacheHttp = src -> vcHttp;
        } else if ( rc == 0 ) {
            rc = r2;
        }
        r2 = VPathAddRef ( src -> vcHttps );
        if ( r2 == 0 ) {
            p -> cacheHttps = src -> vcHttps;
        } else if ( rc == 0 ) {
            rc = r2;
        }
    }
    if ( rc == 0 ) {
        atomic32_set ( & p -> refcount, 1 );
        * self = p;
    } else {
        VPathSetWhack ( p );
    }
    return rc;
}

/* VPathSetList */
rc_t VPathSetListMake ( VPathSetList ** self ) {
    VPathSetList * p = ( VPathSetList * ) calloc ( 1, sizeof * p );
    if ( p == NULL ) {
        return RC ( rcVFS, rcPath, rcAllocating, rcMemory, rcExhausted );
    }

    atomic32_set ( & p -> refcount, 1 );

    assert ( self );

    * self = p;

    return 0;
}

rc_t VPathSetListAddRef ( const VPathSetList * self ) {
    if ( self != NULL ) {
        atomic32_inc ( & ( ( VPathSetList * ) self ) -> refcount );
    }

    return 0;
}

rc_t VPathSetListRelease ( const VPathSetList * cself ) {
    VPathSetList * self = ( VPathSetList * ) cself;

    if ( self != NULL && atomic32_dec_and_test ( & self -> refcount ) ) {
        VectorWhack ( & self -> list, whackVPathSet, NULL );
        memset ( self, 0, sizeof * self );
        free ( self );
    }

    return 0;
}

rc_t VPathSetListAppend ( VPathSetList * self, const VPathSet * set ) {
    assert ( self );

    return VectorAppend ( & self -> list, NULL, set );
}

uint32_t VPathSetListLength ( const VPathSetList * self ) {
    assert ( self );

    return VectorLength ( & self -> list );
}

rc_t VPathSetListGet
    ( const VPathSetList * self, uint32_t idx, const VPathSet ** set )
{
    if ( self == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    }

    if ( set == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    }
    else {
        const VPathSet * s = ( VPathSet * ) VectorGet ( & self -> list, idx );
        if ( s == NULL ) {
            return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
        }
        else {
            rc_t rc = VPathSetAddRef ( s );
            if ( rc == 0 ) {
                * set = s;
            }
            return rc;
        }
    }
}

rc_t VPathSetListGetPath ( const VPathSetList * self, uint32_t idx,
    VRemoteProtocols p, const VPath ** path )
{
    if ( self == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcSelf, rcNull );
    }

    if ( path == NULL ) {
        return RC ( rcVFS, rcQuery, rcExecuting, rcParam, rcNull );
    }
    else {
        const VPathSet * s = ( VPathSet * ) VectorGet ( & self -> list, idx );
        if ( p == 0 ) {
            p = eProtocolHttpHttps;
        }
        if ( s == NULL ) {
            return RC ( rcVFS, rcPath, rcAccessing, rcItem, rcNotFound );
        }
        else {
            const VPath * v = NULL;
            rc_t rc = VPathSetGet ( s, p, & v, NULL );
            if ( rc == 0 ) {
                * path = v;
            }
            return rc;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////