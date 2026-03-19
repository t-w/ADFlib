#include <check.h>
#include <stdlib.h>


#include "adflib.h"
#include "adf_util.h"
#include "test_util.h"


START_TEST( test_check_framework ) {
    ck_assert ( 1 );
}
END_TEST


START_TEST( test_swap_boot )
{
#define BUFSIZE 1024
    uint8_t * const boot = malloc( BUFSIZE );
    pattern_random( boot, BUFSIZE );

    uint8_t * const bootBak = malloc( BUFSIZE );
    memcpy( bootBak, boot, BUFSIZE );

    const uint32_t
        long1 = *(uint32_t *)( boot + 4 ),
        long2 = *(uint32_t *)( boot + 8 );

    adfSwapEndian( boot, ADF_SWBL_BOOT );

    // check swapped things are correct
    ck_assert_uint_eq( long1, swapUint32( *(uint32_t *)( boot + 4 ) ) );
    ck_assert_uint_eq( long2, swapUint32( *(uint32_t *)( boot + 8 ) ) );
    
    // check nothing else was changed
    ck_assert_int_eq( 0, memcmp( boot, bootBak, 4 ) );
    ck_assert_int_eq( 0, memcmp( boot + 12,
                                 bootBak + 12,
                                 BUFSIZE - 12 ) );

    // swap back
    adfSwapEndian( boot, ADF_SWBL_BOOT );

    // check all the same
    ck_assert_int_eq( 0, memcmp( boot, bootBak, BUFSIZE ) );
#undef BUFSIZE
    free( bootBak );
    free( boot );
}
END_TEST

START_TEST( test_swap_root )
{
#define BUFSIZE 512
    uint8_t * const block = malloc( BUFSIZE );
    pattern_random( block, BUFSIZE );

    uint8_t * const blockBak = malloc( BUFSIZE );
    memcpy( blockBak, block, BUFSIZE );

    const unsigned nlongs1 = 108;
    const size_t nlongs1_size = nlongs1 * sizeof( uint32_t );
    uint32_t * const longs1 = malloc( nlongs1_size );
    memcpy( longs1, block, nlongs1_size );

    const unsigned nlongs2 = 10;
    const size_t nlongs2_size = nlongs2 * sizeof( uint32_t );
    uint32_t * const longs2 = malloc( nlongs2_size );
    const size_t longs2_offset = nlongs1 * sizeof( uint32_t ) + 40;
    memcpy( longs2, block + longs2_offset, nlongs2_size );

    adfSwapEndian( block, ADF_SWBL_ROOT );

    // check swapped things are correct
    for ( unsigned i = 0; i < nlongs1; i++ )
        ck_assert_uint_eq( longs1[ i ],
                           swapUint32( ( (uint32_t *) block )[ i ] ) );

    for ( unsigned i = 0; i < nlongs2; i++ )
        ck_assert_uint_eq( longs2[ i ],
                           swapUint32( ( (uint32_t *)( block + longs2_offset ) )[ i ] ) );
    
    // check nothing else was changed
    ck_assert_int_eq( 0, memcmp( block + nlongs1_size,
                                 blockBak + nlongs1_size, 40 ) );

    // swap back
    adfSwapEndian( block, ADF_SWBL_ROOT );

    // check all the same
    ck_assert_int_eq( 0, memcmp( block, blockBak, BUFSIZE ) );

#undef BUFSIZE
    free( longs2 );
    free( longs1 );
    free( blockBak );
    free( block );
}
END_TEST


START_TEST( test_swap_data )
{
#define BUFSIZE 512
    uint8_t * const block = malloc( BUFSIZE );
    pattern_random( block, BUFSIZE );

    uint8_t * const blockBak = malloc( BUFSIZE );
    memcpy( blockBak, block, BUFSIZE );

    const unsigned nlongs1 = 6;
    const size_t nlongs1_size = nlongs1 * sizeof( uint32_t );
    uint32_t * const longs1 = malloc( nlongs1_size );
    memcpy( longs1, block, nlongs1_size );

    adfSwapEndian( block, ADF_SWBL_DATA );

    // check swapped things are correct
    for ( unsigned i = 0; i < nlongs1; i++ )
        ck_assert_uint_eq( longs1[ i ],
                           swapUint32( ( (uint32_t *) block )[ i ] ) );    

    // check nothing else was changed
    ck_assert_int_eq( 0, memcmp( block + nlongs1_size,
                                 blockBak + nlongs1_size, 488 ) );

    // swap back
    adfSwapEndian( block, ADF_SWBL_DATA );

    // check all the same
    ck_assert_int_eq( 0, memcmp( block, blockBak, BUFSIZE ) );

#undef BUFSIZE
    free( longs1 );
    free( blockBak );
    free( block );
}
END_TEST


Suite * adflib_suite( void )
{
    Suite * const suite = suite_create( "adfSwapEndian" );
    
    TCase * tcase = tcase_create( "check framework" );
    tcase_add_test( tcase, test_check_framework );
    suite_add_tcase( suite, tcase );

    tcase = tcase_create( "adflib test_swap_boot" );
    tcase_add_test( tcase, test_swap_boot );
    suite_add_tcase( suite, tcase );

    tcase = tcase_create( "adflib test_swap_root" );
    tcase_add_test( tcase, test_swap_root );
    suite_add_tcase( suite, tcase );

    tcase = tcase_create( "adflib test_swap_data" );
    tcase_add_test( tcase, test_swap_data );
    suite_add_tcase( suite, tcase );

    return suite;
}


int main( void )
{
    Suite * const   suite   = adflib_suite();
    SRunner * const srunner = srunner_create( suite );

    adfLibInit();
    srunner_run_all( srunner, CK_VERBOSE ); //CK_NORMAL );
    adfLibCleanUp();

    const int number_failed = srunner_ntests_failed( srunner );
    srunner_free( srunner );

    return ( number_failed == 0 ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
