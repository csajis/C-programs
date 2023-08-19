/************************************************************************
 *
 * PolynomialOperations.c
 *
 * Programmer: Claudio Saji Santander
 * Compile: gcc -o PolyOp.exe PolynomialOperations.c -lm
 * Execute: ./PolyOp.exe
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct poly {
    unsigned g ;  // degree
    long *c ;  // coefficients
} my_poly ;

#define MAX_GEN 4194304
//#define MAX_GEN 100
#define K 89

void InitPolynomial( my_poly *polynomial ) ;
void FreePolynomial( my_poly *polynomial ) ;
void WritePolynomial( const my_poly polynomial ) ;
void GeneratePolynomial( my_poly *polynomial ) ;
void GenerateAll( my_poly *polynomial , const unsigned degree ) ;
void CopyPolynomial( my_poly *polynomial2 , const my_poly polynomial1 ) ;
void SelectPolynomial( my_poly *polynomial2 , const my_poly polynomial1 , const unsigned maxdegree , const unsigned mindegree ) ;

void SumPolynomials( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
void FixedSum( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
void SubtractPolynomials( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
void FixedSubtract( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
void MultiplyPolynomials1( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
// maximum degrees 1021 (total 2^11-6)
void MultiplyPolynomials2( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
// maximum degrees 524192 (total 2^20-192)
void FixedMultiply( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
// maximum degrees 524192 (total 2^20-192)
void MultiplyXn( my_poly *polynomial2 , const my_poly polynomial , const unsigned n );
void MultiplyXnSum( my_poly *polynomial3 , const my_poly polynomial2 , const my_poly polynomial1 , const unsigned n );
void JoinParts( my_poly *polynomial4 , const my_poly polynomial3 , const my_poly polynomial2 , const my_poly polynomial1 , const unsigned n );
void ClassicStep( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
void KaratsubaStep( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
void Karatsuba( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 );
// permitted degrees (to ensure size of the coefficients) 2^22-1


int main(void)
{
    srand(time(0)); //use current time as seed for random generator
    int i , j , k ;
    unsigned degree ;
    printf( "Number of polynomials: " ) ;
    scanf( "%d" , &k );
    my_poly polynomial1[k] ;
    my_poly polynomial2[k] ;
    my_poly polynomial3 ;
    my_poly polynomial4 ;
    my_poly polynomial5 ;
    for ( i = 0 ; i < k ; i++ )
    {
        InitPolynomial( &(polynomial1[i]) ) ;
        InitPolynomial( &(polynomial2[i]) ) ;
    }
    InitPolynomial( &polynomial3 ) ;
    InitPolynomial( &polynomial4 ) ;
    InitPolynomial( &polynomial5 ) ;
    clock_t time1 , time2 , time3 ;
    
    printf( "degree of the polynomials to generate: " );
    scanf( "%u" , &degree );
    for ( i = 0 ; i < k ; i++ )
    {
        GenerateAll( &(polynomial1[i]) , degree ) ;
        GenerateAll( &(polynomial2[i]) , degree ) ;
    }
//    MultiplyPolynomials1( &polynomial3 , polynomial1[0] , polynomial2[0] ) ;
//    MultiplyPolynomials2( &polynomial3 , polynomial1[0] , polynomial2[0] ) ;
    FixedMultiply( &polynomial3 , polynomial1[0] , polynomial2[0] ) ;
    Karatsuba( &polynomial4 , polynomial1[0] , polynomial2[0] ) ;
    SubtractPolynomials( &polynomial5 , polynomial3 , polynomial4 ) ;
    WritePolynomial( polynomial5 ) ;
    time1 = clock() ;
//    for ( i = 0 ; i < k ; i++ )
//    {
//        for ( j = 0 ; j < k ; j++ )
//        {
//            FixedMultiply( &polynomial3 , polynomial1[i] , polynomial2[j] ) ;
//        }
//    }
    time2 = clock() ;
    for ( i = 0 ; i < k ; i++ )
    {
        for ( j = 0 ; j < k ; j++ )
        {
            Karatsuba( &polynomial3 , polynomial1[i] , polynomial2[j] ) ;
            //KaratsubaStep( &polynomial3 , polynomial1[i] , polynomial2[j] ) ;
        }
    }
    time3 = clock() ;
//    printf("Ratio between times: %f\n" , (((double)time2-(double)time1)/((double)time3-(double)time2)) ) ;
    printf("Tiempo: %f\n" , ( (double)time3-(double)time2 ) / ( (double)CLOCKS_PER_SEC * (double)k * (double)k ) ) ;

    for ( i = 0 ; i < k ; i++ )
    {
        FreePolynomial( &(polynomial1[i]) ) ;
        FreePolynomial( &(polynomial2[i]) ) ;
    }
    FreePolynomial( &polynomial3 ) ;
    FreePolynomial( &polynomial4 ) ;
    FreePolynomial( &polynomial5 ) ;
    return(0);
}




void InitPolynomial( my_poly *polynomial )
{
    (*polynomial).g = 0 ;
    (*polynomial).c = (long *) calloc( 1 , sizeof(long) );
}



void FreePolynomial( my_poly *polynomial )
{
    free( (*polynomial).c );
}



void WritePolynomial( const my_poly polynomial )
{
    unsigned i ;
    unsigned degree = polynomial.g ;
    
    while ( ( polynomial.c[degree] == 0 ) && ( degree > 0 ) )
    {
        degree-- ;
    }
    if ( degree > 1 )
    {
        printf( "%li x^%u" , polynomial.c[degree] , degree );
        for ( i = degree-1 ; i > 1 ; i-- )
        {
            if ( polynomial.c[i] > 0 )
            {
                printf(" + %li x^%u" , polynomial.c[i] , i );
            }
            if ( polynomial.c[i] < 0 )
            {
                printf(" - %li x^%u" , - polynomial.c[i] , i );
            }
        }
        if ( polynomial.c[1] > 0 )
        {
            printf(" + %li x" , polynomial.c[1] );
        }
        if ( polynomial.c[1] < 0 )
        {
            printf(" - %li x" , - polynomial.c[1] );
        }
        if ( polynomial.c[0] > 0 )
        {
            printf(" + %li" , polynomial.c[0] );
        }
        if ( polynomial.c[0] < 0 )
        {
            printf(" - %li" , - polynomial.c[0] );
        }
        printf("\n");
    }
    else
    {
        if ( degree == 1 )
        {
            printf( "%li x" , polynomial.c[1] );
            if ( polynomial.c[0] > 0 )
            {
                printf(" + %li" , polynomial.c[0] );
            }
            if ( polynomial.c[0] < 0 )
            {
                printf(" - %li" , - polynomial.c[0] );
            }
            printf("\n");
        }
        else
        {
            printf("%li\n" , polynomial.c[0] );
        }
    }
}



void GeneratePolynomial( my_poly *polynomial )
{
    unsigned i ;
    unsigned degree ;
    
    printf( "degree del polynomial a generar: " );
    scanf( "%u" , &degree );
    
    (*polynomial).g = degree ;
    (*polynomial).c = (long *) realloc( (*polynomial).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial).c == NULL )
    {
        free( (*polynomial).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= degree ; i++ )
    {
        (*polynomial).c[i] = rand() % MAX_GEN ;
    }
    while ( (*polynomial).c[degree] == 0 )
    {
        (*polynomial).c[degree] = rand() % MAX_GEN ;
    }
}



void GenerateAll( my_poly *polynomial , const unsigned degree )
{
    unsigned i ;
    
    (*polynomial).g = degree ;
    (*polynomial).c = (long *) realloc( (*polynomial).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial).c == NULL )
    {
        free( (*polynomial).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= degree ; i++ )
    {
        (*polynomial).c[i] = rand() % MAX_GEN ;
    }
    while ( (*polynomial).c[degree] == 0 )
    {
        (*polynomial).c[degree] = rand() % MAX_GEN ;
    }
    
}



void CopyPolynomial( my_poly *polynomial2 , const my_poly polynomial1 )
{
    unsigned i ;
    unsigned degree = polynomial1.g ;
    
    while ( ( polynomial1.c[degree] == 0 ) && ( degree > 0 ) )
    {
        degree-- ;
    }
    (*polynomial2).g = degree ;
    (*polynomial2).c = (long *) realloc( (*polynomial2).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial2).c == NULL )
    {
        free( (*polynomial2).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= degree ; i++ )
    {
        (*polynomial2).c[i] = polynomial1.c[i] ;
    }
}




void SelectPolynomial( my_poly *polynomial2 , const my_poly polynomial1 , const unsigned maxdegree , const unsigned mindegree )
{
    unsigned degree , gmax , i ;
    
    if ( polynomial1.g < maxdegree )
    {
        gmax = polynomial1.g ;
    }
    else
    {
        gmax = maxdegree ;
    }
    while ( ( polynomial1.c[gmax] == 0) && ( gmax > 0 ) )
    {
        gmax-- ;
    }
    if ( mindegree > gmax )
    {
        (*polynomial2).g = 0 ;
        (*polynomial2).c = (long *) realloc( (*polynomial2).c , sizeof(long) ) ;
        (*polynomial2).c[1] = 0 ;
    }
    else
    {
        degree = gmax - mindegree ;
        (*polynomial2).g = degree ;
        (*polynomial2).c = (long *) realloc( (*polynomial2).c , ( degree+1 ) * sizeof(long) ) ;
        if ( (*polynomial2).c == NULL )
        {
            free( (*polynomial2).c ) ;
            printf( "Error (re)allocating memory\n" ) ;
            exit( 1 ) ;
        }
        for ( i = 0 ; i <= degree ; i++ )
        {
            (*polynomial2).c[i] = polynomial1.c[i+mindegree] ;
        }
    }
}



void SumPolynomials( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    unsigned i , degree , gmin ;
    int caso = 0 ;

    if ( polynomial1.g > polynomial2.g )
    {
        degree = polynomial1.g ;
        gmin = polynomial2.g ;
        while ( ( polynomial1.c[degree] == 0) && ( degree > gmin ) )
        {
            degree-- ;
        }
        caso = 1 ;
    }
    else
    {
        degree = polynomial2.g ;
        gmin = polynomial1.g ;
        while ( ( polynomial2.c[degree] == 0) && ( degree > gmin ) )
        {
            degree-- ;
        }
        caso = 2 ;
    }
    if ( gmin == degree )
    {
        caso = 0 ;
        while ( ( polynomial1.c[degree] == - polynomial2.c[degree] ) && ( degree > 0 ) )
        {
            degree-- ;
        }
        gmin = degree ;
    }
    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    if ( caso > 0 )
    {
        if ( caso == 1 )
        {
            for ( i = gmin+1 ; i <= degree ; i++ )
            {
                (*polynomial3).c[i] = polynomial1.c[i] ;
            }
        }
        else
        {
            for ( i = gmin+1 ; i <= degree ; i++ )
            {
                (*polynomial3).c[i] = polynomial2.c[i] ;
            }
        }
    }
    for ( i = 0 ; i <= gmin ; i++ )
    {
        (*polynomial3).c[i] = polynomial1.c[i] + polynomial2.c[i] ;
    }
}



void FixedSum( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{ // polynomial1.g = polynomial2.g or polynomial2.g + 1
    unsigned i , degree , gmin ;
    int caso = 0 ;
    
    degree = polynomial1.g ;
    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= polynomial2.g ; i++ )
    {
        (*polynomial3).c[i] = polynomial1.c[i] + polynomial2.c[i] ;
    }
    if ( polynomial2.g < degree )
    {
        (*polynomial3).c[degree] = polynomial1.c[i] ;
    }
}



void SubtractPolynomials( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    unsigned i , degree , gmin ;
    int caso = 0 ;
    
    if ( polynomial1.g > polynomial2.g )
    {
        degree = polynomial1.g ;
        gmin = polynomial2.g ;
        while ( ( polynomial1.c[degree] == 0) && ( degree > gmin ) )
        {
            degree-- ;
        }
        caso = 1 ;
    }
    else
    {
        degree = polynomial2.g ;
        gmin = polynomial1.g ;
        while ( ( polynomial2.c[degree] == 0) && ( degree > gmin ) )
        {
            degree-- ;
        }
        caso = 2 ;
    }
    if ( gmin == degree )
    {
        caso = 0 ;
        while ( ( polynomial1.c[degree] == polynomial2.c[degree] ) && ( degree > 0 ) )
        {
            degree-- ;
        }
        gmin = degree ;
    }
    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    if ( caso > 0 )
    {
        if ( caso == 1 )
        {
            for ( i = gmin+1 ; i <= degree ; i++ )
            {
                (*polynomial3).c[i] = polynomial1.c[i] ;
            }
        }
        else
        {
            for ( i = gmin+1 ; i <= degree ; i++ )
            {
                (*polynomial3).c[i] = - polynomial2.c[i] ;
            }
        }
    }
    for ( i = 0 ; i <= gmin ; i++ )
    {
        (*polynomial3).c[i] = polynomial1.c[i] - polynomial2.c[i] ;
    }
}



void FixedSubtract( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{ // polynomial1.g = polynomial2.g or polynomial2.g + 2
    unsigned i , degree , gmin ;
    int caso = 0 ;
    
    degree = polynomial1.g ;
    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= polynomial2.g ; i++ )
    {
        (*polynomial3).c[i] = polynomial1.c[i] - polynomial2.c[i] ;
    }
    if ( polynomial2.g < degree )
    {
        (*polynomial3).c[degree-1] = polynomial1.c[degree-1] ;
        (*polynomial3).c[degree] = polynomial1.c[degree] ;
    }
}



void MultiplyPolynomials1( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    unsigned i , j , degree ;
    long productos[polynomial1.g+1][polynomial2.g+1] ;
    long acumular[polynomial1.g+polynomial2.g+1];
    
    for ( i = 0 ; i <= polynomial1.g ; i++ )
    {
        for ( j = 0 ; j <= polynomial2.g ; j++ )
        {
            productos[i][j] = polynomial1.c[i] * polynomial2.c[j] ;
        }
    }
    if ( polynomial1.g < polynomial2.g )
    {
        for ( i = 0 ; i <= polynomial1.g ; i++ )
        {
            acumular[i] = productos[i][0] ;
            for ( j = 1 ; j <= i ; j++ )
            {
                acumular[i] += productos[i-j][j] ;
            }
        }
        for ( i = polynomial1.g+1 ; i <= polynomial2.g ; i++ )
        {
            acumular[i] = productos[polynomial1.g][i-polynomial1.g] ;
            for ( j = i-polynomial1.g+1 ; j <= i ; j++ )
            {
                acumular[i] += productos[i-j][j] ;
            }
        }
        for ( i = polynomial2.g+1 ; i <= polynomial1.g+polynomial2.g ; i++ )
        {
            acumular[i] = productos[polynomial1.g][i-polynomial1.g] ;
            for ( j = i-polynomial1.g+1 ; j <= polynomial2.g ; j++ )
            {
                acumular[i] += productos[i-j][j] ;
            }
        }
    }
    else
    {
        for ( i = 0 ; i <= polynomial2.g ; i++ )
        {
            acumular[i] = productos[i][0] ;
            for ( j = 1 ; j <= i ; j++ )
            {
                acumular[i] += productos[i-j][j] ;
            }
        }
        for ( i = polynomial2.g+1 ; i <= polynomial1.g ; i++ )
        {
            acumular[i] = productos[i][0] ;
            for ( j = 1 ; j <= polynomial2.g ; j++ )
            {
                acumular[i] += productos[i-j][j] ;
            }
        }
        for ( i = polynomial1.g+1 ; i <= polynomial1.g+polynomial2.g ; i++ )
        {
            acumular[i] = productos[polynomial1.g][i-polynomial1.g] ;
            for ( j = i-polynomial1.g+1 ; j <= polynomial2.g ; j++ )
            {
                acumular[i] += productos[i-j][j] ;
            }
        }
    }
    
    degree = polynomial1.g + polynomial2.g ;
    while ( ( acumular[degree] == 0) && ( degree > 0 ) )
    {
        (degree)-- ;
    }
    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i= 0 ; i <= degree ; i++ )
    {
        (*polynomial3).c[i] = acumular[i] ;
    }
}



void MultiplyPolynomials2( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    unsigned i , j , degree ;
    long acumular[polynomial1.g+polynomial2.g+1];
    
    for ( j = 0 ; j <= polynomial2.g ; j++ )
    {
        acumular[j] = polynomial1.c[0] * polynomial2.c[j] ;
    }
    for ( i = 1 ; i <= polynomial1.g ; i++ )
    {
        for ( j = 0 ; j < polynomial2.g ; j++ )
        {
            acumular[i+j] += polynomial1.c[i] * polynomial2.c[j] ;
        }
        acumular[i+polynomial2.g] = polynomial1.c[i] * polynomial2.c[polynomial2.g] ;
    }
    degree = polynomial1.g + polynomial2.g ;
    while ( ( acumular[degree] == 0) && ( degree > 0 ) )
    {
        (degree)-- ;
    }
    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= degree ; i++ )
    {
        (*polynomial3).c[i] = acumular[i] ;
    }
}




void FixedMultiply( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    unsigned i , j , degree ;
    
    degree = polynomial1.g + polynomial2.g ;
    long acumular[degree+1];

    for ( j = 0 ; j <= polynomial2.g ; j++ )
    {
        acumular[j] = polynomial1.c[0] * polynomial2.c[j] ;
    }
    for ( i = 1 ; i <= polynomial1.g ; i++ )
    {
        for ( j = 0 ; j < polynomial2.g ; j++ )
        {
            acumular[i+j] += polynomial1.c[i] * polynomial2.c[j] ;
        }
        acumular[i+polynomial2.g] = polynomial1.c[i] * polynomial2.c[polynomial2.g] ;
    }

    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= degree ; i++ )
    {
        (*polynomial3).c[i] = acumular[i] ;
    }
}




void MultiplyXn( my_poly *polynomial2 , const my_poly polynomial , const unsigned n )
{
    unsigned degree , i ;
    
    degree = polynomial.g ;
    if ( ( degree == 0 ) && ( polynomial.c[0] == 0 ) )
    {
        CopyPolynomial( polynomial2 , polynomial ) ;
    }
    else
    {
        (*polynomial2).g = degree + n ;
        (*polynomial2).c = (long *) realloc( (*polynomial2).c , ( degree + n + 1 ) * sizeof(long) ) ;
        if ( (*polynomial2).c == NULL )
        {
            free( (*polynomial2).c ) ;
            printf( "Error (re)allocating memory\n" ) ;
            exit( 1 ) ;
        }
        for ( i = 0 ; i < n ; i++ )
        {
            (*polynomial2).c[i] = 0 ;
        }
        for ( i = 0 ; i <= degree ; i++ )
        {
            (*polynomial2).c[i+n] = polynomial.c[i] ;
        }
    }
}





void MultiplyXnSum( my_poly *polynomial3 , const my_poly polynomial2 , const my_poly polynomial1 , const unsigned n )
{ // polynomial3 = polynomial2 * X^n + polynomial1
    unsigned degree , i ;
    
    degree = polynomial2.g + n ;
    if ( polynomial1.g > degree )
    {
        degree = polynomial1.g ;
    }
    (*polynomial3).g = degree ;
    (*polynomial3).c = (long *) realloc( (*polynomial3).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial3).c == NULL )
    {
        free( (*polynomial3).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i <= polynomial1.g ; i++ )
    {
        (*polynomial3).c[i] = (polynomial1).c[i] ;
    }
    for ( i = n ; i <= polynomial1.g ; i++ )
    {
        (*polynomial3).c[i] += (polynomial2).c[i-n] ;
    }
    for ( i = ( polynomial1.g + 1 ) ; i <= polynomial2.g + n ; i++ )
    {
        (*polynomial3).c[i] = (polynomial2).c[i-n] ;
    }
    
}



void JoinParts( my_poly *polynomial4 , const my_poly polynomial3 , const my_poly polynomial2 , const my_poly polynomial1 , const unsigned n )
{ // polynomial4 = polynomial3 * X^2n + polynomial2 * X^n + polynomial1
    unsigned degree , i ;
    
    degree = polynomial3.g + (2*n) ;
    (*polynomial4).g = degree ;
    (*polynomial4).c = (long *) realloc( (*polynomial4).c , ( degree+1 ) * sizeof(long) ) ;
    if ( (*polynomial4).c == NULL )
    {
        free( (*polynomial4).c ) ;
        printf( "Error (re)allocating memory\n" ) ;
        exit( 1 ) ;
    }
    for ( i = 0 ; i < n ; i++ )
    {
        (*polynomial4).c[i] = (polynomial1).c[i] ;
    }
    for ( i = n ; i <= polynomial1.g ; i++ )
    {
        (*polynomial4).c[i] = (polynomial1).c[i] + (polynomial2).c[i-n] ;
    }
    for ( i = ( polynomial1.g + 1 ) ; i < ( 2*n ) ; i++ )
    {
        (*polynomial4).c[i] = (polynomial2).c[i-n] ;
    }
    for ( i = (2*n) ; i <= (polynomial2.g + n) ; i++ )
    {
        (*polynomial4).c[i] = (polynomial2).c[i-n] + (polynomial3).c[i-(2*n)] ;
    }
    for ( i = ( polynomial2.g + n + 1 ) ; i <= polynomial3.g + (2*n) ; i++ )
    {
        (*polynomial4).c[i] = (polynomial3).c[i-(2*n)] ;
    }
}



void ClassicStep( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    unsigned tamano ;
    my_poly a1 , a0 , b1 , b0 , t0 , t1 , t2 ;
    InitPolynomial( &a1 ) ; InitPolynomial( &a0 ) ;
    InitPolynomial( &b1 ) ; InitPolynomial( &b0 ) ;
    InitPolynomial( &t0 ) ; InitPolynomial( &t1 ) ; InitPolynomial( &t2 ) ;

    if ( polynomial1.g > polynomial2.g )
    {
        tamano = polynomial1.g + 1 ;
    }
    else
    {
        tamano = polynomial2.g + 1 ;
    }
    if ( ( tamano % 2 ) == 1 )
    {
        tamano++ ;
    }
    tamano = tamano / 2 ;
    SelectPolynomial( &a0 , polynomial1 , tamano-1 , 0 ) ;
    SelectPolynomial( &a1 , polynomial1 , polynomial1.g , tamano ) ;
    SelectPolynomial( &b0 , polynomial2 , tamano-1 , 0 ) ;
    SelectPolynomial( &b1 , polynomial2 , polynomial2.g , tamano ) ;
    
    MultiplyPolynomials2( &t0 , a0 , b1 ) ;
    MultiplyPolynomials2( &t1 , a1 , b0 ) ;
    SumPolynomials( &t2 , t0 , t1 ) ;
    MultiplyPolynomials2( &t0 , a1 , b1 ) ;
    MultiplyXn( &t1 , t0 , tamano ) ;
    SumPolynomials( &t0 , t2 , t1 ) ;
    MultiplyXn( &t1 , t0 , tamano ) ;
    MultiplyPolynomials2( &t0 , a0 , b0 ) ;
    SumPolynomials( polynomial3 , t0 , t1 ) ;
    
    FreePolynomial( &a1 ) ;
    FreePolynomial( &a0 ) ;
    FreePolynomial( &b1 ) ;
    FreePolynomial( &b0 ) ;
    FreePolynomial( &t0 ) ;
    FreePolynomial( &t1 ) ;
    FreePolynomial( &t2 ) ;
}



void KaratsubaStep( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    unsigned tamano ;
    my_poly a1 , a0 , b1 , b0 , c0 , t0 , t1 ;
    InitPolynomial( &a1 ) ; InitPolynomial( &a0 ) ;
    InitPolynomial( &b1 ) ; InitPolynomial( &b0 ) ;
    InitPolynomial( &c0 ) ;
    InitPolynomial( &t0 ) ; InitPolynomial( &t1 ) ;
    
    if ( polynomial1.g > polynomial2.g )
    {
        tamano = polynomial1.g + 1 ;
    }
    else
    {
        tamano = polynomial2.g + 1 ;
    }
    if ( ( tamano % 2 ) == 1 )
    {
        tamano++ ;
    }
    tamano = tamano / 2 ;
    SelectPolynomial( &a0 , polynomial1 , tamano-1 , 0 ) ;
    SelectPolynomial( &a1 , polynomial1 , polynomial1.g , tamano ) ;
    SelectPolynomial( &b0 , polynomial2 , tamano-1 , 0 ) ;
    SelectPolynomial( &b1 , polynomial2 , polynomial2.g , tamano ) ;
    FixedMultiply( &t0 , a0 , b0 ) ; // a0 * b0
    FixedMultiply( &t1 , a1 , b1 ) ; // a1 * b1
    FixedSum( &c0 , a0 , a1 ) ; // a0 + a1
    FixedSum( &a1 , b0 , b1 ) ; // b0 + b1
    FixedMultiply( &a0 , c0 , a1 ) ; // ( a0 + a1 ) * ( b0 + b1 )
    FixedSubtract( &b0 , a0 , t0 ) ; // ( a0 + a1 ) * ( b0 + b1 ) - a0 * b0
    FixedSubtract( &a1 , b0 , t1 ) ; // a0 * b1 + a1 * b0
    JoinParts( polynomial3 , t1 , a1 , t0 , tamano ) ;
    
    FreePolynomial( &a1 ) ;
    FreePolynomial( &a0 ) ;
    FreePolynomial( &b1 ) ;
    FreePolynomial( &b0 ) ;
    FreePolynomial( &c0 ) ;
    FreePolynomial( &t0 ) ;
    FreePolynomial( &t1 ) ;
}




void Karatsuba( my_poly *polynomial3 , const my_poly polynomial1 , const my_poly polynomial2 )
{
    if ( ( polynomial1.g > K ) && ( polynomial2.g > K ) )
    {
        unsigned tamano ;
        my_poly a1 , a0 , b1 , b0 , c0 , t0 , t1 ;
        InitPolynomial( &a1 ) ; InitPolynomial( &a0 ) ;
        InitPolynomial( &b1 ) ; InitPolynomial( &b0 ) ;
        InitPolynomial( &c0 ) ;
        InitPolynomial( &t0 ) ; InitPolynomial( &t1 ) ;
        if ( polynomial1.g > polynomial2.g )
        {
            tamano = polynomial1.g + 1 ;
        }
        else
        {
            tamano = polynomial2.g + 1 ;
        }
        if ( ( tamano % 2 ) == 1 )
        {
            tamano++ ;
        }
        tamano = tamano / 2 ;
        SelectPolynomial( &a0 , polynomial1 , tamano-1 , 0 ) ;
        SelectPolynomial( &a1 , polynomial1 , polynomial1.g , tamano ) ;
        SelectPolynomial( &b0 , polynomial2 , tamano-1 , 0 ) ;
        SelectPolynomial( &b1 , polynomial2 , polynomial2.g , tamano ) ;
        Karatsuba( &t0 , a0 , b0 ) ; // a0 * b0
        Karatsuba( &t1 , a1 , b1 ) ; // a1 * b1
        FixedSum( &c0 , a0 , a1 ) ; // a0 + a1
        FixedSum( &a1 , b0 , b1 ) ; // b0 + b1
        Karatsuba( &a0 , c0 , a1 ) ; // ( a0 + a1 ) * ( b0 + b1 )
        FixedSubtract( &b0 , a0 , t0 ) ; // ( a0 + a1 ) * ( b0 + b1 ) - a0 * b0
        FixedSubtract( &a1 , b0 , t1 ) ; // a0 * b1 + a1 * b0
        JoinParts( polynomial3 , t1 , a1 , t0 , tamano ) ;
        
        FreePolynomial( &a1 ) ;
        FreePolynomial( &a0 ) ;
        FreePolynomial( &b1 ) ;
        FreePolynomial( &b0 ) ;
        FreePolynomial( &c0 ) ;
        FreePolynomial( &t0 ) ;
        FreePolynomial( &t1 ) ;
    }
    else
    {
        FixedMultiply( polynomial3 , polynomial2 , polynomial1 ) ;
    }
}


