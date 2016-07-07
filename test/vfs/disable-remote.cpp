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

#include <kfg/config.h> /* KConfig */
#include <kfs/directory.h> /* KDirectory */
#include <klib/text.h> /* String */
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <vfs/manager.h> /* VFSManager */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/path.h> /* VPath */
#include <vfs/resolver.h> /* VResolverCacheEnable */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

rc_t StringRelease ( const String * self ) { free ( (void *) self ); return 0; }

TEST_SUITE(DisableSuite);

typedef enum {
    eNone                            =      0,
    eCgi                             =      1,
    eAux                             =      2,

    eRemoteDisableFalse              =      4,
    eRemoteDisableTrue               =      8,

    eRemoteMainDisableTrue           =     16,
    eRemoteMainCgiDisableFalse       =     32,
    eRemoteMainCgiDisableTrue        =     64,

    eRemoteAuxDisableTrue            =    128,
    eRemoteAuxNcbiDisableFalse       =    256,
    eRemoteAuxNcbiDisableTrue        =    512,
} E;

class SET {
public:
    static rc_t set ( KConfig * self, int e ) {
        if ( e == 0 ){
            return 0;
        }
        rc_t rc = 0;
        if ( rc == 0 && e & eCgi ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/main/CGI/resolver-cgi",
                "http://www.ncbi.nlm.nih.gov/Traces/names/names.cgi" );
        }
        if ( rc == 0 && e & eAux ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/aux/NCBI/root",
                "http://ftp-trace.ncbi.nlm.nih.gov/sra" );
        }
        if ( rc == 0 && e & eRemoteDisableFalse ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/disabled", "false" );
        }
        if ( rc == 0 && e & eRemoteDisableTrue ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/disabled", "true" );
        }
        if ( rc == 0 && e & eRemoteMainDisableTrue ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/main/disabled", "true" );
        }
        if ( rc == 0 && e & eRemoteMainCgiDisableFalse ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/main/CGI/disabled", "false" );
        }
        if ( rc == 0 && e & eRemoteMainCgiDisableTrue ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/main/CGI/disabled", "true" );
        }
        if ( rc == 0 && e & eRemoteAuxDisableTrue ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/aux/disabled", "true" );
        }
        if ( rc == 0 && e & eRemoteAuxNcbiDisableFalse ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/aux/NCBI/disabled", "false" );
        }
        if ( rc == 0 && e & eRemoteAuxNcbiDisableTrue ) {
            rc = KConfigWriteString ( self,
               "/repository/remote/aux/NCBI/disabled", "true" );
        }
        return rc;
    }
};

class Test : protected ncbi::NK::TestCase {
    TestCase * _dad;

public:
    Test ( TestCase * dad, bool resolve, int v )
        : TestCase ( dad -> GetName () ), _dad ( dad )
    {
        rc_t rc = 0;
        KDirectory * native  = NULL;
        REQUIRE_RC ( KDirectoryNativeDir ( & native ) );
        const KDirectory * dir = NULL;
        REQUIRE_RC
            ( KDirectoryOpenDirRead ( native, &dir, false, "disable-remote" ) );
        KConfig * cfg = NULL;
        REQUIRE_RC ( KConfigMake ( & cfg, dir ) );
        REQUIRE_RC ( SET :: set ( cfg, v ) );
// KConfigPrint(cfg, 0);
        VFSManager * vfs = NULL;
        REQUIRE_RC ( VFSManagerMakeFromKfg ( & vfs, cfg ) );
        VResolver * resolver = NULL;
        REQUIRE_RC ( VFSManagerGetResolver ( vfs, & resolver ) );
        VPath * query = NULL;
        REQUIRE_RC ( VFSManagerMakePath ( vfs, & query, "SRR053325" ) );
        const VPath * remote = NULL;
        if ( resolve ) {
            REQUIRE_RC ( VResolverQuery
                ( resolver, eProtocolHttp, query, NULL, & remote, NULL ) );
            const String * s = NULL;
            REQUIRE_RC ( VPathMakeString ( remote, & s ) );
            String e;
            if ( v & eCgi ) {
                CONST_STRING ( & e,
                    "http://sra-download.ncbi.nlm.nih.gov/srapub/SRR053325" );
            } else {
                CONST_STRING ( & e,
                    "http://ftp-trace.ncbi.nlm.nih.gov/sra/sra-instant/reads/"
                           "ByRun/sra/SRR/SRR053/SRR053325/SRR053325.sra" );
            }
            REQUIRE_EQ ( StringCompare ( s, & e ), 0 );
            RELEASE ( String, s );
        } else
            REQUIRE_RC_FAIL ( VResolverQuery
                ( resolver, eProtocolHttp, query, NULL, & remote, NULL ) );
        RELEASE ( VPath, query );
        RELEASE ( VPath, query );
        RELEASE ( VResolver, resolver );
        RELEASE ( VFSManager, vfs );
        RELEASE ( KConfig, cfg );
        RELEASE ( KDirectory, dir );
        RELEASE ( KDirectory, native );
        REQUIRE_RC ( rc );
    }
    ~Test ( void ) {
        assert( _dad );
        _dad->ErrorCounterAdd(GetErrorCounter());
    }
};

class Disabled : private Test {
public:
    Disabled (TestCase * dad, int v = eNone ) : Test ( dad, false, v ) {}
};

class Enabled : private Test {
public:
    Enabled (TestCase * dad, int v ) : Test ( dad, true, v ) {}
};

TEST_CASE ( INCOMPLETE ) {
    Disabled ( this );
}

TEST_CASE ( CGI ) {
    Enabled ( this, eCgi );
}

TEST_CASE ( CGI_DISABLE ) {
    Disabled ( this, eCgi | eRemoteDisableTrue );
}
TEST_CASE ( CGI_DISABLE_NOT ) {
    Enabled ( this, eCgi  | eRemoteDisableFalse );
}

TEST_CASE ( CGI_DisableMain ) { // /repository/remote/main/disabled is ignored
    Enabled ( this, eCgi  | eRemoteMainDisableTrue );
}

TEST_CASE ( CGI_DisableCgi ) {
    Disabled ( this, eCgi  | eRemoteMainCgiDisableTrue );
}
TEST_CASE ( CGI_EnableCgi ) {
    Enabled ( this, eCgi  | eRemoteMainCgiDisableFalse );
}

TEST_CASE ( CGI_EnableReoote_DisableCgi ) {
    Disabled ( this, eCgi  | eRemoteDisableFalse | eRemoteMainCgiDisableTrue );
}

TEST_CASE ( AUX ) {
    Enabled ( this, eAux );
}

TEST_CASE ( AUX_DISABLE ) {
    Disabled ( this, eAux | eRemoteDisableTrue );
}
TEST_CASE ( AUX_DISABLE_NOT ) {
    Enabled ( this, eAux  | eRemoteDisableFalse );
}

TEST_CASE ( AUX_DisableAux ) { // /repository/remote/aux/disabled is ignored
    Enabled ( this, eAux  | eRemoteAuxDisableTrue );
}

TEST_CASE ( AUX_DisableNcbi ) {
    Disabled ( this, eAux  | eRemoteAuxNcbiDisableTrue );
}
TEST_CASE ( AUX_EnableNcbi ) {
    Enabled ( this, eAux  | eRemoteAuxNcbiDisableFalse );
}

TEST_CASE ( AUX_EnableReoote_DisableNcbi ) {
    Disabled ( this, eAux  | eRemoteDisableFalse | eRemoteAuxNcbiDisableTrue );
}

////////////////////////////////////////////////////////////////////////////////

extern "C" {
    ver_t CC KAppVersion ( void ) {
        return 0;
    }
    rc_t CC KMain ( int argc, char * argv [] ) {
        return DisableSuite ( argc, argv );
    }
}
