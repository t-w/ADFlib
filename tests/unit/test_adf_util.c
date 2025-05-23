#include <check.h>
#include <stdio.h>
#include <stdlib.h>

//#include "adflib.h"
#include "adf_util.h"

#ifndef BUILDING_WITH_CMAKE
#include "config.h"   // include config. header generated by autotools
#endif


START_TEST ( test_check_framework )
{
    ck_assert( 1 );
}
END_TEST

#ifndef HAVE_STPNCPY
// test only custom implementation
START_TEST(test_stpncpy)
{
    const char input[] = "whatever";
    char output[32];

    // copy 0 chars
    memset( output, 0, 32 );
    char* endp = stpncpy( output, input, 0 );
    ck_assert( endp == &output[0] );
    for ( unsigned i = 0; i < 32; i++ )
        ck_assert( output[i] == 0 );

    // copy all
    memset( output, 0, 32 );
    endp = stpncpy( output, input, 8 );
    ck_assert( endp == &output[8] );
    ck_assert( strcmp(input, output) == 0 );
    ck_assert( strncmp(input, output, 8) == 0 );
    for ( unsigned i = 8; i < 32; i++ )
        ck_assert( output[i] == 0 );

    // copy 4 chars
    memset(output, 0, 32);
    endp = stpncpy( output, input, 4 );
    ck_assert( endp == &output[4] );
    ck_assert( strncmp( input, output, 4 ) == 0 );
    for ( unsigned i = 4; i < 32; i++ )
        ck_assert( output[i] == 0 );

    // more than string len
    memset( output, 0, 32 );
    endp = stpncpy( output, input, 16 );
    ck_assert( endp == &output[8] );
    ck_assert( strcmp(input, output) == 0 );
    ck_assert( strncmp(input, output, 8) == 0 );
    for ( unsigned i = 8; i < 32; i++ )
        ck_assert( output[i] == 0 );
}
END_TEST
#endif


Suite * adflib_suite(void)
{
    Suite * const suite = suite_create( "adf util" );
    
    TCase * tcase = tcase_create( "check framework" );
    tcase_add_test( tcase, test_check_framework );
    suite_add_tcase( suite, tcase );

#ifndef HAVE_STPNCPY
    tcase = tcase_create( "test stpncpy" );
    tcase_add_test( tcase, test_stpncpy );
    suite_add_tcase( suite, tcase );
#endif

    return suite;
}


int main(void)
{
    Suite * const suite = adflib_suite();
    SRunner * const srunner = srunner_create( suite );

    srunner_run_all( srunner, CK_VERBOSE ); //CK_NORMAL );
    const int nfailed = srunner_ntests_failed( srunner );
    srunner_free( srunner );
    return ( nfailed == 0 ) ?
        EXIT_SUCCESS :
        EXIT_FAILURE;
}
