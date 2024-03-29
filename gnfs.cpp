
#include <gnfs.hpp>
#include <polynomial_selection.hpp>
#include <factor_base.hpp>
#include <sieve.hpp>
#include <linear_algebra.hpp>
#include <square_root.hpp>

// {{{ polynomial_save()
void polynomial_save(const Polynomial &polynomial)
{
   std::ofstream file("polynomial.gnfs");
   file << polynomial.d << std::endl;
   file << polynomial.f << std::endl; 
   file << polynomial.m << std::endl;
}
// }}}

// {{{ polynomial_read()
bool polynomial_read(Polynomial &polynomial)
{
   std::ifstream file("polynomial.gnfs");

   file >> polynomial.d;
   file >> polynomial.f;
   file >> polynomial.m;

   if( polynomial.d>0 && NTL::deg(polynomial.f)>0 && polynomial.m>0)
   {
      std::cout << "Reading polynomial from file" << std::endl;
      return true;
   }

   return false;
}
// }}}


// {{{ calc_B()
// Prime Numbers: A Computational Perspective - Crandall & Pomerance
// B = exp( (8/9)^(1/3) * (ln n)^(1/3) * (lnln n)^(2/3) )
int
calc_B(NTL::ZZ n)
{
   NTL::RR lnn;
   NTL::RR lnlnn;
   NTL::RR A, B, C;
   int result = 0;

   lnn = NTL::log(n);
   lnlnn = NTL::log(lnn);

   A = NTL::pow(NTL::to_RR((double)8 / 9), NTL::to_RR((double)1 / 3));
   B = NTL::pow(lnn, NTL::to_RR((double)1 / 3));
   C = NTL::pow(lnlnn, NTL::to_RR((double)2 / 3));

   result = 10*NTL::to_int(exp(A * B * C));

   return result;
}

// }}}

// {{{ calc_U()
// Factoring Integers With The Number Field Sieve 
// Buhler & Lenstra & Pomerance
// u = exp( 1/2 * ( d*(lnd)+sqrt( (d*lnd)^2 + 4*(ln(n^(1/d)))*lnln(n^(1/d))) ))
int
calc_U(NTL::ZZ & n, int d)
{
   int result = 0;

   NTL::RR lnd;
   NTL::RR lnn1d;
   NTL::RR lnlnn1d;
   NTL::RR A, B, C, D;

   lnd = NTL::log(NTL::to_RR(d));
   lnn1d = NTL::log(NTL::pow(NTL::to_RR(n), NTL::to_RR((double)1 / d)));
   lnlnn1d = NTL::log(lnn1d);

   double e = 0;

   A = (double)((1 + e) / 2);
   B = d * lnd;
   C = (d * lnd) * (d * lnd);
   D = 4 * lnd * lnlnn1d;

   result = to_int(NTL::exp(A * (B + NTL::sqrt(C + D))));

   return result;
}

// }}}

// {{{ extract_little_factors()
NTL::ZZ extract_little_factors(NTL::ZZ & n, const char *primes)
{
   std::ifstream fin(primes);

   NTL::ZZ prime;

   fin >> prime;
   while(!fin.eof() && prime < 1000)
   {
      if(n % prime == 0)
      {
         std::cout << "factor: " << prime << std::endl;
         n /= prime;
      }
      fin >> prime;
   }

   return n;
}

// }}}

// ----------------------------------------------------------------------------
// MAIN - General Number Field Sieve
// ----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   using namespace std;
   using namespace NTL;

   int t0, t1;
   int u;
   int v;
   int pairs_needed;
   NTL::ZZ xZ;
   NTL::ZZ yZ;      
   std::ifstream fin;
   std::string line = "  -------------------------------------"
                      "-------------------------------------  ";

	Target target;


   if(argc != 3)
   {
      cout << "Usage: gnfs [N] [primes] " << endl;
      exit(1);
   }

   target.n = NTL::to_ZZ(argv[1]);
	target.nbits = NTL::NumBits(target.n);
   NTL::RR::SetPrecision(NTL::NumBits(target.n));

   /*
   target.n = extract_little_factors(target.n, argv[2]);
   if(NTL::ProbPrime(target.n))
   {
      std::cout << "factor: " << target.n << std::endl;
      return 0;
   }
   if(target.n == 1)
      return 0;
   */

	// Create polynomial
	Polynomial polynomial;
	polynomial.d=3;


   target.digits = (int)(log(target.n) / log(10)) + 1;
   target.t = calc_U(target.n, polynomial.d);
   target.C = calc_B(target.n);
   
   std::cout << std::endl;
   std::cout << line << std::endl;
   std::cout << "\tGeneral Number Field Sieve (GNFS)" << std::endl;
   std::cout << line << std::endl;

   std::cout << "\tTarget Number: " << target.n << std::endl;
   std::cout << "\tDigits: " << target.digits << std::endl;
   std::cout << "\tNum Bits: " << target.nbits << std::endl;
   std::cout << "\tdegree:  " << polynomial.d << std::endl;
   std::cout << "\tFB Size: " << target.t << std::endl;
   std::cout << "\tSive Interval: " << target.C << std::endl;


   // ------------------------------------------------------------------------
   // FASE 1: Seleccion de Polinomio
   // ------------------------------------------------------------------------
   std::cout << std::endl;
   std::cout << line << std::endl;
   std::cout << "\tPolynomial Selection" << std::endl;
   std::cout << line << std::endl;
   
   t0 = time(NULL);

   //if(!polynomial_read(polynomial))
      polynomial_selection(polynomial, target, argv[2]);

   //polynomial_save(polynomial);

	std::cout << "\tPolynomial: " << polynomial.f << std::endl;
	std::cout << "\tDegree: " << polynomial.d << std::endl;
	std::cout << "\tm: " << polynomial.m << std::endl;

   t1 = time(NULL);
   printf("\ttime: %d\n", t1-t0);



   // ------------------------------------------------------------------------
   // FASE 2: Crea las Bases de Factores
   // ------------------------------------------------------------------------
   std::cout << std::endl;
   std::cout << line << std::endl;
   std::cout << "\tMake Factor Base" << std::endl;
   std::cout << line << std::endl;

   t0 = time(NULL);

   FactorBase fb;

   // Crea la "Rational Factor Base"
   fb.make_RFB(polynomial, target, argv[2]);
   std::cout << "\tRFB: " << fb.RFB.size() << " elements" << std::endl;

   // Crea la "Algebraic Factor Base"
   fb.make_AFB(polynomial, target, argv[2]);
   std::cout << "\tAFB: " << fb.AFB.size() << " elements" << std::endl;

   // Crea la "Quadratic Factor Base"
   fb.make_QFB(target, polynomial, fb.AFB[fb.AFB.size()-1], argv[2]);
   std::cout << "\tQCB: " << fb.QCB.size() << " elements" << std::endl;

   v = target.digits;
   u = polynomial.d * target.t;

   t1 = time(NULL);
   printf("\ttime: %d\n", t1-t0);


   // ------------------------------------------------------------------------
   // FASE 3: Criba
   // ------------------------------------------------------------------------
   std::cout << std::endl;
   std::cout << line << std::endl;
   std::cout << "\tSieve" << std::endl;
   std::cout << line << std::endl;

   t0 = time(NULL);

   std::vector<int> av; 
   std::vector<int> bv; 
   pairs_needed = target.t + u + v + 2;
   sieve(polynomial, target, fb, pairs_needed, av, bv);

   t1 = time(NULL);
   printf("\n\n\ttime: %d\n", t1-t0);
 

   // ------------------------------------------------------------------------
   // FASE 4: Algebra Lineal
   // ------------------------------------------------------------------------
   std::cout << std::endl;
   std::cout << line << std::endl;
   std::cout << "\tLinear Algebra" << std::endl;
   std::cout << line << std::endl;
   t0 = time(NULL);

   Matrix matrix(pairs_needed, pairs_needed);
   linear_algebra(polynomial, target, fb, matrix, av, bv);

   t1 = time(NULL);
   printf("\n\n\ttime: %d\n", t1-t0);

	
   // ------------------------------------------------------------------------
   // FASE 5: Raiz Cuadrada
   // ------------------------------------------------------------------------
   std::cout << std::endl;
   std::cout << line << std::endl;
   std::cout << "\tSquare Root" << std::endl;
   std::cout << line << std::endl;

   t0 = time(NULL);

   square_root(polynomial, target, matrix, pairs_needed, pairs_needed-1, 
      fb, av, bv, xZ, yZ);

   t1 = time(NULL);
   printf("\ttime: %d\n", t1-t0);


   // ------------------------------------------------------------------------
   // FASE 6: Factores
   // ------------------------------------------------------------------------
   std::cout << std::endl;
   std::cout << line << std::endl;
   std::cout << "\tFactors: " << std::endl;
   std::cout << line << std::endl;

   cout << "\tfactor: " <<  GCD(xZ-yZ, target.n) << endl;
   cout << "\tfactor: " << GCD(xZ+yZ, target.n) << endl;
   std::cout << std::endl;

   return (EXIT_SUCCESS);
}


