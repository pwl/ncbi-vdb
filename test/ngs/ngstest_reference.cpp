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

/**
* Unit tests for NGS C interface, reference accessions
*/

#include "ngs_c_fixture.hpp"

#include <string.h>

#include <ktst/unit_test.hpp>

#include <kfc/xc.h>

#include "NGS_ReferenceSequence.h"
#include "NGS_String.h"

using namespace std;
using namespace ncbi::NK;

TEST_SUITE(NgsReferenceTestSuite);

FIXTURE_TEST_CASE(SRA_Reference_Open, NGS_C_Fixture)
{
    ENTRY;
    const char* SRA_Reference = "NC_000001.10";
    NGS_ReferenceSequence * ref = NGS_ReferenceSequenceMake ( ctx, SRA_Reference );
    REQUIRE ( ! FAILED () );
    REQUIRE_NOT_NULL ( ref );
    NGS_ReferenceSequenceRelease ( ref, ctx );
    EXIT;
}

FIXTURE_TEST_CASE(SRA_Reference_Open_FailsOnNonReference, NGS_C_Fixture)
{
    ENTRY;
    const char* SRA_Accession = "SRR000001";
    REQUIRE_NULL ( NGS_ReferenceSequenceMake ( ctx, SRA_Accession ) );
    REQUIRE ( ctx_xc_isa ( ctx, xcTableOpenFailed )  );
    REQUIRE_FAILED ();
    EXIT;
}

FIXTURE_TEST_CASE(EBI_Reference_Open_EBI_MD5, NGS_C_Fixture)
{
/* The following request should success it order to this test to work:
http://www.ebi.ac.uk/ena/cram/md5/ffd6aeffb54ade3d28ec7644afada2e9
Otherwise CALL_TO_EBI_RESOLVER_FAILS is set
and this test is expected to fail */
    const bool CALL_TO_EBI_RESOLVER_FAILS = false;

    ENTRY;
    const char* EBI_Accession = "ffd6aeffb54ade3d28ec7644afada2e9";

    if ( CALL_TO_EBI_RESOLVER_FAILS ) {
    }

    NGS_ReferenceSequence * ref = NGS_ReferenceSequenceMake ( ctx, EBI_Accession );

    if ( CALL_TO_EBI_RESOLVER_FAILS ) {
        REQUIRE ( FAILED () );
        REQUIRE_NULL ( ref );
        LOG(ncbi::NK::LogLevel::e_error,
            "CANNOT TEST EBI ACCESSION BECAUSE THEIR SITE DOES NOT RESPOND!\n");
        LOG(ncbi::NK::LogLevel::e_error, "NOW EXPECTING AN ERROR MESSAGE ...");
        return;
    }

    REQUIRE ( ! FAILED () );
    REQUIRE_NOT_NULL ( ref );

    NGS_String* bases = NGS_ReferenceSequenceGetBases ( ref, ctx, 4, 16 );

    REQUIRE ( strcmp (NGS_StringData ( bases, ctx ), "CTTTCTGACCGAAATT") == 0 );

    REQUIRE_EQ ( NGS_ReferenceSequenceGetLength(ref, ctx), (uint64_t)784333 );

    // to suppress the warning of unused function - call toString
    toString ( bases, ctx );

    NGS_StringRelease ( bases, ctx );
    NGS_ReferenceSequenceRelease ( ref, ctx );
    EXIT;
}

FIXTURE_TEST_CASE(EBI_Reference_Open_EBI_ACC, NGS_C_Fixture)
{
    ENTRY;
    const char* EBI_Accession = "U12345.1";
    NGS_ReferenceSequence * ref = NGS_ReferenceSequenceMake ( ctx, EBI_Accession );
    REQUIRE ( ! FAILED () );
    REQUIRE_NOT_NULL ( ref );

    NGS_String* bases = NGS_ReferenceSequenceGetBases ( ref, ctx, 4, 16 );
    REQUIRE ( strcmp (NGS_StringData ( bases, ctx ), "CCGCTATCAATATACT") == 0 );

    REQUIRE_EQ ( NGS_ReferenceSequenceGetLength(ref, ctx), (uint64_t)426 );

    NGS_StringRelease ( bases, ctx );
    NGS_ReferenceSequenceRelease ( ref, ctx );
    EXIT;
}


//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-ngs_sra";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t m_coll=NgsReferenceTestSuite(argc, argv);
    return m_coll;
}

}

