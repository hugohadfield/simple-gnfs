
#ifndef __GNFS_SIEVE_HPP__
#define __GNFS_SIEVE_HPP__

#include <gnfs.hpp>
#include <factor_base.hpp>


NTL::ZZ algebraic_norm(Polynomial &polynomial, int a, int b);

void sieve(Polynomial &polynomial, Target &target, FactorBase &fb,   
   int pairs_needed, std::vector<int> &av, std::vector<int> &bv);


#endif

