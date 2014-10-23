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

#ifndef _zehr_h_
#define _zehr_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   This file contains methods and definitions, which
  ))))  should be extended and reimplemented later
 ((((     All names in that file ends with _ZHR
  ))))
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))  Universal method which should load config from resource
 ((*/

struct KConfig;

XFS_EXTERN rc_t CC XFS_LoadConfig_ZHR (
                                    const char * Resource,
                                    const struct KConfig ** Konfig
                                    );


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct KNamelist;

/*))
 ((   Simple string tokeniser: splits string by separator and fills
  ))  KNamelist, trims 'isspace' from begin and end of strings
 ((*/
XFS_EXTERN rc_t CC XFS_SimpleTokenize_ZHR (
                                        const char * SimpleString,
                                        char Separator,
                                        struct KNamelist ** Tokens
                                        );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 ((   That is correct comparision function needed for BSTree operations
  ))
 ((*/
XFS_EXTERN int CC XFS_StringCompare4BST_ZHR (
                                    const char * Str1,
                                    const char * Str2
                                    );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct VPath;

/*))
  ||   Prints VPath to buffer, and, if there was error of reading,
  ||   or Path is NULL, the Filler value will be copied to buffer.
  ||   Filler should be 0 terminated string, or NULL If Filler is
  ||   NULL, the "NULL" will be copied to buffer then.
  ((*/
XFS_EXTERN rc_t CC XFS_ReadVPath_ZHR (
                                    const struct VPath * Path,
                                    char * Buffer,
                                    size_t BufferSize,
                                    const char * Filler
                                    );
/*))
  ||   Prints VPath Uri to buffer, and, if there was error of reading,
  ||   or Path is NULL, the Filler value will be copied to buffer.
  ||   Filler should be 0 terminated string, or NULL If Filler is
  ||   NULL, the "NULL" will be copied to buffer then.
  ((*/
XFS_EXTERN rc_t CC XFS_ReadVUri_ZHR (
                                    const struct VPath * Path,
                                    char * Buffer,
                                    size_t BufferSize,
                                    const char * Filler
                                    );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _zehr_h_ */