#ifndef PhyloCTMCSiteHomogeneousNucleotide_H
#define PhyloCTMCSiteHomogeneousNucleotide_H

#include "AbstractPhyloCTMCSiteHomogeneous.h"
#include "DnaState.h"
#include "RateMatrix.h"
#include "RbVector.h"
#include "TopologyNode.h"
#include "TransitionProbabilityMatrix.h"
#include "TypedDistribution.h"

namespace RevBayesCore {
    
    template<class charType>
    class PhyloCTMCSiteHomogeneousNucleotide : public AbstractPhyloCTMCSiteHomogeneous<charType> {
        
    public:
        PhyloCTMCSiteHomogeneousNucleotide(const TypedDagNode< Tree > *t, bool c, size_t nSites, bool amb, bool internal, bool gapmatch);
        virtual                                            ~PhyloCTMCSiteHomogeneousNucleotide(void);                                                                   //!< Virtual destructor
        
        // public member functions
        PhyloCTMCSiteHomogeneousNucleotide*                 clone(void) const;                                                                          //!< Create an independent clone
        
    protected:
        
        void                                                computeInternalNodeLikelihood(const TopologyNode &n, size_t nIdx, size_t l, size_t r);
        void                                                computeInternalNodeLikelihood(const TopologyNode &n, size_t nIdx, size_t l, size_t r,  size_t m);
        void                                                computeRootLikelihood( size_t root, size_t left, size_t right);
        void                                                computeRootLikelihood( size_t root, size_t left, size_t right, size_t middle);
        void                                                computeTipLikelihood(const TopologyNode &node, size_t nIdx);

        void                                                computeInternalNodeLikelihoodNode(const TopologyNode &n, size_t nIdx, size_t l, size_t r);
        void                                                computeInternalNodeLikelihoodNode(const TopologyNode &n, size_t nIdx, size_t l, size_t r,  size_t m);
        void                                                computeRootLikelihoodNode( size_t root, size_t left, size_t right);
        void                                                computeRootLikelihoodNode( size_t root, size_t left, size_t right, size_t middle);

        
    private:        
        
    };
    
}


#include "ConstantNode.h"
#include "DiscreteCharacterState.h"
#include "RateMatrix_JC.h"
#include "RandomNumberFactory.h"
#include "RbMathLogic.h"

#include <cmath>
#include <cstring>
#if defined (SSE_ENABLED)
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#elif defined ( AVX_ENABLED )
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>
#endif

template<class charType>
RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::PhyloCTMCSiteHomogeneousNucleotide(const TypedDagNode<Tree> *t, bool c, size_t nSites, bool amb, bool internal, bool gapmatch) : AbstractPhyloCTMCSiteHomogeneous<charType>(  t, 4, 1, c, nSites, amb, internal, gapmatch )
{
    
}

template<class charType>
RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::~PhyloCTMCSiteHomogeneousNucleotide( void )
{
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
    
}


template<class charType>
RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>* RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::clone( void ) const
{
    
    return new PhyloCTMCSiteHomogeneousNucleotide<charType>( *this );
}


template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeInternalNodeLikelihood(const TopologyNode &node, size_t node_index, size_t left, size_t right)
{
    
    // compute the transition probability matrix
//    this->updateTransitionProbabilities( node_index );
    size_t pmat_offset = this->active_pmatrices[node_index] * this->active_P_matrix_offset + node_index * this->pmat_node_offset;
    
#   if defined ( SSE_ENABLED )
    
    double* p_left   = this->partial_likelihoods + this->active_likelihood[left]*this->active_likelihood_offset + left*this->node_offset;
    double* p_right  = this->partial_likelihoods + this->active_likelihood[right]*this->active_likelihood_offset + right*this->node_offset;
    double* p_node   = this->partial_likelihoods + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;
    //    __m128d* p_left   = (__m128d *) this->partial_likelihoods + this->active_likelihood[left]*this->active_likelihood_offset + left*this->node_offset;
    //    __m128d* p_right  = (__m128d *) this->partial_likelihoods + this->active_likelihood[right]*this->active_likelihood_offset + right*this->node_offset;
    //    __m128d* p_node   = (__m128d *) this->partial_likelihoods + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;
    
#   elif defined ( AVX_ENABLED )

    double* p_left   = this->partial_likelihoods + this->active_likelihood[left]*this->active_likelihood_offset + left*this->node_offset;
    double* p_right  = this->partial_likelihoods + this->active_likelihood[right]*this->active_likelihood_offset + right*this->node_offset;
    double* p_node   = this->partial_likelihoods + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;

    double* tmp_ac = new double[4];
    double* tmp_gt = new double[4];
//    double tmp_ac[4];
//    double tmp_gt[4];
    
    
#   else

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left  = this->partial_likelihoods + this->active_likelihood[left]*this->active_likelihood_offset + left*this->node_offset;
    const double*   p_right = this->partial_likelihoods + this->active_likelihood[right]*this->active_likelihood_offset + right*this->node_offset;
    double*         p_node  = this->partial_likelihoods + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;

#   endif
        
    bool test_underflow  = RbSettings::userSettings().getUseScaling() == true;
    bool test_this_node  = ((node_index+1) % RbSettings::userSettings().getScalingDensity() == 0);
    bool scale_threshold = RbSettings::userSettings().getScalingMethod() == "threshold";
    bool scale_per_mixture  = RbSettings::userSettings().getScalingPerMixture();

    test_underflow = test_underflow && scale_per_mixture;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const double* tp_begin = this->pmatrices[pmat_offset + mixture].theMatrix;
        
        // get the pointers to the likelihood for this mixture category
        size_t offset = mixture*this->mixture_offset;
        
#       if defined ( SSE_ENABLED )
        
        double*          p_site_mixture          = p_node + offset;
        const double*    p_site_mixture_left     = p_left + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
        __m128d tp_a_ac = _mm_load_pd(tp_begin);
        __m128d tp_a_gt = _mm_load_pd(tp_begin+2);
        __m128d tp_c_ac = _mm_load_pd(tp_begin+4);
        __m128d tp_c_gt = _mm_load_pd(tp_begin+6);
        __m128d tp_g_ac = _mm_load_pd(tp_begin+8);
        __m128d tp_g_gt = _mm_load_pd(tp_begin+10);
        __m128d tp_t_ac = _mm_load_pd(tp_begin+12);
        __m128d tp_t_gt = _mm_load_pd(tp_begin+14);
        
#       elif defined ( AVX_ENABLED )
        
        double*          p_site_mixture          = p_node + offset;
        const double*    p_site_mixture_left     = p_left + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
        __m256d tp_a = _mm256_load_pd(tp_begin);
        __m256d tp_c = _mm256_load_pd(tp_begin+4);
        __m256d tp_g = _mm256_load_pd(tp_begin+8);
        __m256d tp_t = _mm256_load_pd(tp_begin+12);
        
#       else

        double*          p_site_mixture          = p_node + offset;
        const double*    p_site_mixture_left     = p_left + offset;
        const double*    p_site_mixture_right    = p_right + offset;

#       endif

        // compute the per site probabilities
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            
#           if defined ( SSE_ENABLED )
            
            __m128d a01 = _mm_load_pd(p_site_mixture_left);
            __m128d a23 = _mm_load_pd(p_site_mixture_left+2);
            
            __m128d b01 = _mm_load_pd(p_site_mixture_right);
            __m128d b23 = _mm_load_pd(p_site_mixture_right+2);
            
            __m128d p01 = _mm_mul_pd(a01,b01);
            __m128d p23 = _mm_mul_pd(a23,b23);
            
            __m128d a_ac = _mm_mul_pd(p01, tp_a_ac   );
            __m128d a_gt = _mm_mul_pd(p23, tp_a_gt );
            __m128d a_acgt = _mm_hadd_pd(a_ac,a_gt);
            
            __m128d c_ac = _mm_mul_pd(p01, tp_c_ac );
            __m128d c_gt = _mm_mul_pd(p23, tp_c_gt );
            __m128d c_acgt = _mm_hadd_pd(c_ac,c_gt);
            
            __m128d ac = _mm_hadd_pd(a_acgt,c_acgt);
            _mm_store_pd(p_site_mixture,ac);
            
            
            __m128d g_ac = _mm_mul_pd(p01, tp_g_ac  );
            __m128d g_gt = _mm_mul_pd(p23, tp_g_gt );
            __m128d g_acgt = _mm_hadd_pd(g_ac,g_gt);
            
            __m128d t_ac = _mm_mul_pd(p01, tp_t_ac );
            __m128d t_gt = _mm_mul_pd(p23, tp_t_gt );
            __m128d t_acgt = _mm_hadd_pd(t_ac,t_gt);
            
            __m128d gt = _mm_hadd_pd(g_acgt,t_acgt);
            _mm_store_pd(p_site_mixture+2,gt);
 
#           elif defined ( AVX_ENABLED )
 
            __m256d a = _mm256_load_pd(p_site_mixture_left);
            __m256d b = _mm256_load_pd(p_site_mixture_right);
            __m256d p = _mm256_mul_pd(a,b);
            
            __m256d a_acgt = _mm256_mul_pd(p, tp_a );
            __m256d c_acgt = _mm256_mul_pd(p, tp_c );
            __m256d g_acgt = _mm256_mul_pd(p, tp_g );
            __m256d t_acgt = _mm256_mul_pd(p, tp_t );
            
            __m256d ac   = _mm256_hadd_pd(a_acgt,c_acgt);
            __m256d gt   = _mm256_hadd_pd(g_acgt,t_acgt);
            
            
            _mm256_store_pd(tmp_ac,ac);
            _mm256_store_pd(tmp_gt,gt);
            
            p_site_mixture[0] = tmp_ac[0] + tmp_ac[2];
            p_site_mixture[1] = tmp_ac[1] + tmp_ac[3];
            p_site_mixture[2] = tmp_gt[0] + tmp_gt[2];
            p_site_mixture[3] = tmp_gt[1] + tmp_gt[3];

#           else

            double p0 = p_site_mixture_left[0] * p_site_mixture_right[0];
            double p1 = p_site_mixture_left[1] * p_site_mixture_right[1];
            double p2 = p_site_mixture_left[2] * p_site_mixture_right[2];
            double p3 = p_site_mixture_left[3] * p_site_mixture_right[3];
            
            double sum = p0 * tp_begin[0];
            sum += p1 * tp_begin[1];
            sum += p2 * tp_begin[2];
            sum += p3 * tp_begin[3];
            
            p_site_mixture[0] = sum;
            
            sum = p0 * tp_begin[4];
            sum += p1 * tp_begin[5];
            sum += p2 * tp_begin[6];
            sum += p3 * tp_begin[7];
            
            p_site_mixture[1] = sum;
            
            sum = p0 * tp_begin[8];
            sum += p1 * tp_begin[9];
            sum += p2 * tp_begin[10];
            sum += p3 * tp_begin[11];
            
            p_site_mixture[2] = sum;
            
            sum = p0 * tp_begin[12];
            sum += p1 * tp_begin[13];
            sum += p2 * tp_begin[14];
            sum += p3 * tp_begin[15];
            
            p_site_mixture[3] = sum;

#           endif
            
            if ( test_underflow )
            {
                if ( test_this_node == false )
                {
                    this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site];
                }
                else
                {
                    double max = ( p_site_mixture[1] > p_site_mixture[0] ? p_site_mixture[1] : p_site_mixture[0] );
                    max = ( p_site_mixture[2] > max ? p_site_mixture[2] : max );
                    max = ( p_site_mixture[3] > max ? p_site_mixture[3] : max );
                    if ( scale_threshold == false )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] - log(max);
                        p_site_mixture[0] /= max;
                        p_site_mixture[1] /= max;
                        p_site_mixture[2] /= max;
                        p_site_mixture[3] /= max;
                    }
                    else if ( max < RbConstants::SCALING_THRESHOLD )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + 1;
                        p_site_mixture[0] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[1] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[2] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[3] /= RbConstants::SCALING_THRESHOLD;
                    }
                    else
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site];
                    }
                }
            }
            
            // increment the pointers to the next site
            p_site_mixture_left+=this->site_offset; p_site_mixture_right+=this->site_offset; p_site_mixture+=this->site_offset;

                        
        } // end-for over all sites (=patterns)
        
    } // end-for over all mixtures (=rate-categories)

# if defined ( AVX_ENABLED )
    delete[] tmp_ac;
    delete[] tmp_gt;
# endif
    
}


template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeInternalNodeLikelihood(const TopologyNode &node, size_t node_index, size_t left, size_t right, size_t middle)
{
    
    // compute the transition probability matrix
//     this->updateTransitionProbabilities( node_index );
    size_t pmat_offset = this->active_pmatrices[node_index] * this->active_P_matrix_offset + node_index * this->pmat_node_offset;
    
    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left      = this->partial_likelihoods + this->active_likelihood[left]*this->active_likelihood_offset + left*this->node_offset;
    const double*   p_middle    = this->partial_likelihoods + this->active_likelihood[middle]*this->active_likelihood_offset + middle*this->node_offset;
    const double*   p_right     = this->partial_likelihoods + this->active_likelihood[right]*this->active_likelihood_offset + right*this->node_offset;
    double*         p_node      = this->partial_likelihoods + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;
    
    
    bool test_underflow  = RbSettings::userSettings().getUseScaling() == true;
    bool test_this_node  = ( (node_index+1) % RbSettings::userSettings().getScalingDensity() == 0);
    bool scale_threshold = RbSettings::userSettings().getScalingMethod() == "threshold";
    bool scale_per_mixture  = RbSettings::userSettings().getScalingPerMixture();

    test_underflow = test_underflow && scale_per_mixture;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // the transition probability matrix for this mixture category
//         const double* tp_begin = this->transition_prob_matrices[mixture].theMatrix;
        const double* tp_begin = this->pmatrices[pmat_offset + mixture].theMatrix;
        
        // get the pointers to the likelihood for this mixture category
        size_t offset = mixture*this->mixture_offset;
        
#       if defined ( SSE_ENABLED )
        
        double*          p_site_mixture          = p_node + offset;
        const double*    p_site_mixture_left     = p_left + offset;
        const double*    p_site_mixture_middle   = p_middle + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
        __m128d tp_a_ac = _mm_load_pd(tp_begin);
        __m128d tp_a_gt = _mm_load_pd(tp_begin+2);
        __m128d tp_c_ac = _mm_load_pd(tp_begin+4);
        __m128d tp_c_gt = _mm_load_pd(tp_begin+6);
        __m128d tp_g_ac = _mm_load_pd(tp_begin+8);
        __m128d tp_g_gt = _mm_load_pd(tp_begin+10);
        __m128d tp_t_ac = _mm_load_pd(tp_begin+12);
        __m128d tp_t_gt = _mm_load_pd(tp_begin+14);
        
#       elif defined ( AVX_ENABLED )
        
        double*          p_site_mixture          = p_node + offset;
        const double*    p_site_mixture_left     = p_left + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
        __m256d tp_a = _mm256_load_pd(tp_begin);
        __m256d tp_c = _mm256_load_pd(tp_begin+4);
        __m256d tp_g = _mm256_load_pd(tp_begin+8);
        __m256d tp_t = _mm256_load_pd(tp_begin+12);
        
#       else
        
        double*          p_site_mixture          = p_node + offset;
        const double*    p_site_mixture_left     = p_left + offset;
        const double*    p_site_mixture_middle   = p_middle + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
#       endif
        
        // compute the per site probabilities
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            
#           if defined ( SSE_ENABLED )
            
            __m128d a01 = _mm_load_pd(p_site_mixture_left);
            __m128d a23 = _mm_load_pd(p_site_mixture_left+2);
            
            __m128d b01 = _mm_load_pd(p_site_mixture_middle);
            __m128d b23 = _mm_load_pd(p_site_mixture_middle+2);
            
            __m128d c01 = _mm_load_pd(p_site_mixture_right);
            __m128d c23 = _mm_load_pd(p_site_mixture_right+2);
            
            __m128d tmp_p01 = _mm_mul_pd(a01,b01);
            __m128d p01 = _mm_mul_pd(tmp_p01,c01);
            __m128d tmp_p23 = _mm_mul_pd(a23,b23);
            __m128d p23 = _mm_mul_pd(tmp_p23,c23);
            
            __m128d a_ac = _mm_mul_pd(p01, tp_a_ac   );
            __m128d a_gt = _mm_mul_pd(p23, tp_a_gt );
            __m128d a_acgt = _mm_hadd_pd(a_ac,a_gt);
            
            __m128d c_ac = _mm_mul_pd(p01, tp_c_ac );
            __m128d c_gt = _mm_mul_pd(p23, tp_c_gt );
            __m128d c_acgt = _mm_hadd_pd(c_ac,c_gt);
            

            //            *p_site_mixture = _mm_hadd_pd(a_acgt,c_acgt);
            __m128d ac = _mm_hadd_pd(a_acgt,c_acgt);
            _mm_store_pd(p_site_mixture,ac);
            
            
            __m128d g_ac = _mm_mul_pd(p01, tp_g_ac  );
            __m128d g_gt = _mm_mul_pd(p23, tp_g_gt );
            __m128d g_acgt = _mm_hadd_pd(g_ac,g_gt);
            
            __m128d t_ac = _mm_mul_pd(p01, tp_t_ac );
            __m128d t_gt = _mm_mul_pd(p23, tp_t_gt );
            __m128d t_acgt = _mm_hadd_pd(t_ac,t_gt);
            
            //            p_site_mixture[2] = _mm_hadd_pd(g_acgt,t_acgt);
            __m128d gt = _mm_hadd_pd(g_acgt,t_acgt);
            _mm_store_pd(p_site_mixture+2,gt);
            
#           elif defined ( AVX_ENABLED )
            
            __m256d a = _mm256_load_pd(p_site_mixture_left);
            __m256d b = _mm256_load_pd(p_site_mixture_right);
            __m256d p = _mm_mul_pd(a,b);
            
            __m256d a_acgt = _mm256_mul_pd(p, tp_a );
            __m256d c_acgt = _mm256_mul_pd(p, tp_c );
            __m256d g_acgt = _mm256_mul_pd(p, tp_g );
            __m256d t_acgt = _mm256_mul_pd(p, tp_t );
            
            __m256d ac   = _mm256_hadd_pd(a_acgt,c_acgt);
            __m256d gt   = _mm256_hadd_pd(g_acgt,t_acgt)
            
            __m256d acgt = _mm256_hadd_pd(ac,gt);
            
            _mm256_store_pd(p_site_mixture,acgt);
            
#           else
            
            double p0 = p_site_mixture_left[0] * p_site_mixture_middle[0] * p_site_mixture_right[0];
            double p1 = p_site_mixture_left[1] * p_site_mixture_middle[1] * p_site_mixture_right[1];
            double p2 = p_site_mixture_left[2] * p_site_mixture_middle[2] * p_site_mixture_right[2];
            double p3 = p_site_mixture_left[3] * p_site_mixture_middle[3] * p_site_mixture_right[3];
            
            double sum = p0 * tp_begin[0];
            sum += p1 * tp_begin[1];
            sum += p2 * tp_begin[2];
            sum += p3 * tp_begin[3];
            
            p_site_mixture[0] = sum;
            
            sum = p0 * tp_begin[4];
            sum += p1 * tp_begin[5];
            sum += p2 * tp_begin[6];
            sum += p3 * tp_begin[7];
            
            p_site_mixture[1] = sum;
            
            sum = p0 * tp_begin[8];
            sum += p1 * tp_begin[9];
            sum += p2 * tp_begin[10];
            sum += p3 * tp_begin[11];
            
            p_site_mixture[2] = sum;
            
            sum = p0 * tp_begin[12];
            sum += p1 * tp_begin[13];
            sum += p2 * tp_begin[14];
            sum += p3 * tp_begin[15];
            
            p_site_mixture[3] = sum;
            
#           endif
            
            
            if ( test_underflow )
            {
                if ( test_this_node == false )
                {
                    this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site];
                }
                else
                {
                    double max = ( p_site_mixture[1] > p_site_mixture[0] ? p_site_mixture[1] : p_site_mixture[0] );
                    max = ( p_site_mixture[2] > max ? p_site_mixture[2] : max );
                    max = ( p_site_mixture[3] > max ? p_site_mixture[3] : max );
                    if ( scale_threshold == false )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site] - log(max);
                        p_site_mixture[0] /= max;
                        p_site_mixture[1] /= max;
                        p_site_mixture[2] /= max;
                        p_site_mixture[3] /= max;
                    }
                    else if ( max < RbConstants::SCALING_THRESHOLD )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site] + 1;
                        p_site_mixture[0] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[1] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[2] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[3] /= RbConstants::SCALING_THRESHOLD;
                    }
                    else
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site];
                    }
                }
            }
            
            // increment the pointers to the next site
            p_site_mixture_left+=this->site_offset; p_site_mixture_middle+=this->site_offset; p_site_mixture_right+=this->site_offset; p_site_mixture+=this->site_offset;
            
            
        } // end-for over all sites (=patterns)
        
    } // end-for over all mixtures (=rate-categories)
    
}



template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeInternalNodeLikelihoodNode(const TopologyNode &node, size_t node_index, size_t left, size_t right)
{
    
    // compute the transition probability matrix
    size_t pmat_offset_left  = this->active_pmatrices[left]  * this->active_P_matrix_offset + left  * this->pmat_node_offset;
    size_t pmat_offset_right = this->active_pmatrices[right] * this->active_P_matrix_offset + right * this->pmat_node_offset;

#   if defined ( SSE_ENABLED )
    
    double* p_left   = this->partial_likelihoods_node + this->active_likelihood[left]       * this->active_likelihood_offset + left       * this->node_offset;
    double* p_right  = this->partial_likelihoods_node + this->active_likelihood[right]      * this->active_likelihood_offset + right      * this->node_offset;
    double* p_node   = this->partial_likelihoods_node + this->active_likelihood[node_index] * this->active_likelihood_offset + node_index * this->node_offset;
    //    __m128d* p_left   = (__m128d *) this->partial_likelihoods + this->active_likelihood[left]*this->active_likelihood_offset + left*this->node_offset;
    //    __m128d* p_right  = (__m128d *) this->partial_likelihoods + this->active_likelihood[right]*this->active_likelihood_offset + right*this->node_offset;
    //    __m128d* p_node   = (__m128d *) this->partial_likelihoods + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;
    
#   elif defined ( AVX_ENABLED )

    double* p_left   = this->partial_likelihoods_node + this->active_likelihood[left]       * this->active_likelihood_offset + left       * this->node_offset;
    double* p_right  = this->partial_likelihoods_node + this->active_likelihood[right]      * this->active_likelihood_offset + right      * this->node_offset;
    double* p_node   = this->partial_likelihoods_node + this->active_likelihood[node_index] * this->active_likelihood_offset + node_index * this->node_offset;

    double* tmp_ac = new double[4];
    double* tmp_gt = new double[4];
//    double tmp_ac[4];
//    double tmp_gt[4];
    
    
#   else

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left  = this->partial_likelihoods + this->active_likelihood[left]       * this->active_likelihood_offset + left       * this->node_offset;
    const double*   p_right = this->partial_likelihoods + this->active_likelihood[right]      * this->active_likelihood_offset + right      * this->node_offset;
    double*         p_node  = this->partial_likelihoods + this->active_likelihood[node_index] * this->active_likelihood_offset + node_index * this->node_offset;

#   endif

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const double* tp_begin_left  = this->pmatrices[pmat_offset_left  + mixture].theMatrix;
        const double* tp_begin_right = this->pmatrices[pmat_offset_right + mixture].theMatrix;

        // get the pointers to the likelihood for this mixture category
        size_t offset = mixture*this->mixture_offset;
        
#       if defined ( SSE_ENABLED )
        
        double*          p_site_mixture          = p_node  + offset;
        const double*    p_site_mixture_left     = p_left  + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
        __m128d tp_a_ac_left = _mm_load_pd(tp_begin_left);
        __m128d tp_a_gt_left = _mm_load_pd(tp_begin_left+2);
        __m128d tp_c_ac_left = _mm_load_pd(tp_begin_left+4);
        __m128d tp_c_gt_left = _mm_load_pd(tp_begin_left+6);
        __m128d tp_g_ac_left = _mm_load_pd(tp_begin_left+8);
        __m128d tp_g_gt_left = _mm_load_pd(tp_begin_left+10);
        __m128d tp_t_ac_left = _mm_load_pd(tp_begin_left+12);
        __m128d tp_t_gt_left = _mm_load_pd(tp_begin_left+14);
        
        __m128d tp_a_ac_right = _mm_load_pd(tp_begin_right);
        __m128d tp_a_gt_right = _mm_load_pd(tp_begin_right+2);
        __m128d tp_c_ac_right = _mm_load_pd(tp_begin_right+4);
        __m128d tp_c_gt_right = _mm_load_pd(tp_begin_right+6);
        __m128d tp_g_ac_right = _mm_load_pd(tp_begin_right+8);
        __m128d tp_g_gt_right = _mm_load_pd(tp_begin_right+10);
        __m128d tp_t_ac_right = _mm_load_pd(tp_begin_right+12);
        __m128d tp_t_gt_right = _mm_load_pd(tp_begin_right+14);

        
#       elif defined ( AVX_ENABLED )
        
        double*          p_site_mixture          = p_node  + offset;
        const double*    p_site_mixture_left     = p_left  + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
        __m256d tp_a = _mm256_load_pd(tp_begin);
        __m256d tp_c = _mm256_load_pd(tp_begin+4);
        __m256d tp_g = _mm256_load_pd(tp_begin+8);
        __m256d tp_t = _mm256_load_pd(tp_begin+12);
        
#       else

        double*          p_site_mixture          = p_node  + offset;
        const double*    p_site_mixture_left     = p_left  + offset;
        const double*    p_site_mixture_right    = p_right + offset;

#       endif

        // compute the per site probabilities
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            
#           if defined ( SSE_ENABLED )
            
//            __m128d a01 = _mm_load_pd(p_site_mixture_left);
//            __m128d a23 = _mm_load_pd(p_site_mixture_left+2);
//            
//            __m128d b01 = _mm_load_pd(p_site_mixture_right);
//            __m128d b23 = _mm_load_pd(p_site_mixture_right+2);
//            
//            __m128d p01 = _mm_mul_pd(a01,b01);
//            __m128d p23 = _mm_mul_pd(a23,b23);
//            
//            __m128d a_ac = _mm_mul_pd(p01, tp_a_ac   );
//            __m128d a_gt = _mm_mul_pd(p23, tp_a_gt );
//            __m128d a_acgt = _mm_hadd_pd(a_ac,a_gt);
//            
//            __m128d c_ac = _mm_mul_pd(p01, tp_c_ac );
//            __m128d c_gt = _mm_mul_pd(p23, tp_c_gt );
//            __m128d c_acgt = _mm_hadd_pd(c_ac,c_gt);
//            
//            __m128d ac = _mm_hadd_pd(a_acgt,c_acgt);
//            _mm_store_pd(p_site_mixture,ac);
//            
//            
//            __m128d g_ac = _mm_mul_pd(p01, tp_g_ac  );
//            __m128d g_gt = _mm_mul_pd(p23, tp_g_gt );
//            __m128d g_acgt = _mm_hadd_pd(g_ac,g_gt);
//            
//            __m128d t_ac = _mm_mul_pd(p01, tp_t_ac );
//            __m128d t_gt = _mm_mul_pd(p23, tp_t_gt );
//            __m128d t_acgt = _mm_hadd_pd(t_ac,t_gt);
//            
//            __m128d gt = _mm_hadd_pd(g_acgt,t_acgt);
//            _mm_store_pd(p_site_mixture+2,gt);
 
#           elif defined ( AVX_ENABLED )
 
            __m256d a = _mm256_load_pd(p_site_mixture_left);
            __m256d b = _mm256_load_pd(p_site_mixture_right);
            __m256d p = _mm256_mul_pd(a,b);
            
            __m256d a_acgt = _mm256_mul_pd(p, tp_a );
            __m256d c_acgt = _mm256_mul_pd(p, tp_c );
            __m256d g_acgt = _mm256_mul_pd(p, tp_g );
            __m256d t_acgt = _mm256_mul_pd(p, tp_t );
            
            __m256d ac   = _mm256_hadd_pd(a_acgt,c_acgt);
            __m256d gt   = _mm256_hadd_pd(g_acgt,t_acgt);
            
            
            _mm256_store_pd(tmp_ac,ac);
            _mm256_store_pd(tmp_gt,gt);
            
            p_site_mixture[0] = tmp_ac[0] + tmp_ac[2];
            p_site_mixture[1] = tmp_ac[1] + tmp_ac[3];
            p_site_mixture[2] = tmp_gt[0] + tmp_gt[2];
            p_site_mixture[3] = tmp_gt[1] + tmp_gt[3];

#           else
            
            double sum_left = p_site_mixture_left[0] * tp_begin_left[0];
            sum_left += p_site_mixture_left[1] * tp_begin_left[1];
            sum_left += p_site_mixture_left[2] * tp_begin_left[2];
            sum_left += p_site_mixture_left[3] * tp_begin_left[3];
            
            double sum_right = p_site_mixture_right[0] * tp_begin_right[0];
            sum_right += p_site_mixture_right[1] * tp_begin_right[1];
            sum_right += p_site_mixture_right[2] * tp_begin_right[2];
            sum_right += p_site_mixture_right[3] * tp_begin_right[3];
            
            p_site_mixture[0] = sum_left * sum_right;
            
            
            sum_left  = p_site_mixture_left[0] * tp_begin_left[4];
            sum_left += p_site_mixture_left[1] * tp_begin_left[5];
            sum_left += p_site_mixture_left[2] * tp_begin_left[6];
            sum_left += p_site_mixture_left[3] * tp_begin_left[7];
            
            sum_right  = p_site_mixture_right[0] * tp_begin_right[4];
            sum_right += p_site_mixture_right[1] * tp_begin_right[5];
            sum_right += p_site_mixture_right[2] * tp_begin_right[6];
            sum_right += p_site_mixture_right[3] * tp_begin_right[7];
            
            p_site_mixture[1] = sum_left * sum_right;
            
            
            sum_left  = p_site_mixture_left[0] * tp_begin_left[8];
            sum_left += p_site_mixture_left[1] * tp_begin_left[9];
            sum_left += p_site_mixture_left[2] * tp_begin_left[10];
            sum_left += p_site_mixture_left[3] * tp_begin_left[11];
            
            sum_right  = p_site_mixture_right[0] * tp_begin_right[8];
            sum_right += p_site_mixture_right[1] * tp_begin_right[9];
            sum_right += p_site_mixture_right[2] * tp_begin_right[10];
            sum_right += p_site_mixture_right[3] * tp_begin_right[11];
            
            p_site_mixture[2] = sum_left * sum_right;
            
            
            sum_left  = p_site_mixture_left[0] * tp_begin_left[12];
            sum_left += p_site_mixture_left[1] * tp_begin_left[13];
            sum_left += p_site_mixture_left[2] * tp_begin_left[14];
            sum_left += p_site_mixture_left[3] * tp_begin_left[15];
            
            sum_right  = p_site_mixture_right[0] * tp_begin_right[12];
            sum_right += p_site_mixture_right[1] * tp_begin_right[13];
            sum_right += p_site_mixture_right[2] * tp_begin_right[14];
            sum_right += p_site_mixture_right[3] * tp_begin_right[15];
            
            p_site_mixture[3] = sum_left * sum_right;

#           endif
                    
            // increment the pointers to the next site
            p_site_mixture_left+=this->site_offset; p_site_mixture_right+=this->site_offset; p_site_mixture+=this->site_offset;

                        
        } // end-for over all sites (=patterns)
        
    } // end-for over all mixtures (=rate-categories)

# if defined ( AVX_ENABLED )
    delete[] tmp_ac;
    delete[] tmp_gt;
# endif
    
}


template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeInternalNodeLikelihoodNode(const TopologyNode &node, size_t node_index, size_t left, size_t right, size_t middle)
{
    
    // compute the transition probability matrix
    size_t pmat_offset_left  = this->active_pmatrices[left]  * this->active_P_matrix_offset + node_index * this->pmat_node_offset;
    size_t pmat_offset_right = this->active_pmatrices[right] * this->active_P_matrix_offset + node_index * this->pmat_node_offset;

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left      = this->partial_likelihoods_node + this->active_likelihood[left]*this->active_likelihood_offset + left*this->node_offset;
    const double*   p_middle    = this->partial_likelihoods_node + this->active_likelihood[middle]*this->active_likelihood_offset + middle*this->node_offset;
    const double*   p_right     = this->partial_likelihoods_node + this->active_likelihood[right]*this->active_likelihood_offset + right*this->node_offset;
    double*         p_node      = this->partial_likelihoods_node + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const double* tp_begin_left  = this->pmatrices[pmat_offset_left  + mixture].theMatrix;
        const double* tp_begin_right = this->pmatrices[pmat_offset_right + mixture].theMatrix;
        
        // get the pointers to the likelihood for this mixture category
        size_t offset = mixture*this->mixture_offset;
        
#       if defined ( SSE_ENABLED )
        
        double*          p_site_mixture          = p_node   + offset;
        const double*    p_site_mixture_left     = p_left   + offset;
        const double*    p_site_mixture_middle   = p_middle + offset;
        const double*    p_site_mixture_right    = p_right  + offset;
        
        __m128d tp_a_ac_left = _mm_load_pd(tp_begin_left);
        __m128d tp_a_gt_left = _mm_load_pd(tp_begin_left+2);
        __m128d tp_c_ac_left = _mm_load_pd(tp_begin_left+4);
        __m128d tp_c_gt_left = _mm_load_pd(tp_begin_left+6);
        __m128d tp_g_ac_left = _mm_load_pd(tp_begin_left+8);
        __m128d tp_g_gt_left = _mm_load_pd(tp_begin_left+10);
        __m128d tp_t_ac_left = _mm_load_pd(tp_begin_left+12);
        __m128d tp_t_gt_left = _mm_load_pd(tp_begin_left+14);
        
        __m128d tp_a_ac_right = _mm_load_pd(tp_begin_right);
        __m128d tp_a_gt_right = _mm_load_pd(tp_begin_right+2);
        __m128d tp_c_ac_right = _mm_load_pd(tp_begin_right+4);
        __m128d tp_c_gt_right = _mm_load_pd(tp_begin_right+6);
        __m128d tp_g_ac_right = _mm_load_pd(tp_begin_right+8);
        __m128d tp_g_gt_right = _mm_load_pd(tp_begin_right+10);
        __m128d tp_t_ac_right = _mm_load_pd(tp_begin_right+12);
        __m128d tp_t_gt_right = _mm_load_pd(tp_begin_right+14);
        
#       elif defined ( AVX_ENABLED )
        
        double*          p_site_mixture          = p_node  + offset;
        const double*    p_site_mixture_left     = p_left  + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        
        __m256d tp_a = _mm256_load_pd(tp_begin);
        __m256d tp_c = _mm256_load_pd(tp_begin+4);
        __m256d tp_g = _mm256_load_pd(tp_begin+8);
        __m256d tp_t = _mm256_load_pd(tp_begin+12);
        
#       else
        
        double*          p_site_mixture          = p_node   + offset;
        const double*    p_site_mixture_left     = p_left   + offset;
        const double*    p_site_mixture_middle   = p_middle + offset;
        const double*    p_site_mixture_right    = p_right  + offset;
        
#       endif
        
        // compute the per site probabilities
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            
#           if defined ( SSE_ENABLED )
            
            __m128d a01 = _mm_load_pd(p_site_mixture_left);
            __m128d a23 = _mm_load_pd(p_site_mixture_left+2);
            
            __m128d b01 = _mm_load_pd(p_site_mixture_middle);
            __m128d b23 = _mm_load_pd(p_site_mixture_middle+2);
            
            __m128d c01 = _mm_load_pd(p_site_mixture_right);
            __m128d c23 = _mm_load_pd(p_site_mixture_right+2);
            
            __m128d tmp_p01 = _mm_mul_pd(a01,b01);
            __m128d p01 = _mm_mul_pd(tmp_p01,c01);
            __m128d tmp_p23 = _mm_mul_pd(a23,b23);
            __m128d p23 = _mm_mul_pd(tmp_p23,c23);
            
            __m128d a_ac_left = _mm_mul_pd(p01, tp_a_ac_left   );
            __m128d a_gt_left = _mm_mul_pd(p23, tp_a_gt_left );
            __m128d a_acgt_left = _mm_hadd_pd(a_ac_left,a_gt_left);
            
            __m128d c_ac_left = _mm_mul_pd(p01, tp_c_ac_left );
            __m128d c_gt_left = _mm_mul_pd(p23, tp_c_gt_left );
            __m128d c_acgt_left = _mm_hadd_pd(c_ac_left,c_gt_left);
            

            //            *p_site_mixture = _mm_hadd_pd(a_acgt,c_acgt);
            __m128d ac = _mm_hadd_pd(a_acgt_left,c_acgt_left);
            _mm_store_pd(p_site_mixture,ac);
            
            
            __m128d g_ac_left = _mm_mul_pd(p01, tp_g_ac_left  );
            __m128d g_gt_left = _mm_mul_pd(p23, tp_g_gt_left );
            __m128d g_acgt_left = _mm_hadd_pd(g_ac_left,g_gt_left);
            
            __m128d t_ac_left = _mm_mul_pd(p01, tp_t_ac_left );
            __m128d t_gt_left = _mm_mul_pd(p23, tp_t_gt_left );
            __m128d t_acgt_left = _mm_hadd_pd(t_ac_left,t_gt_left);
            
            //            p_site_mixture[2] = _mm_hadd_pd(g_acgt,t_acgt);
            __m128d gt_left = _mm_hadd_pd(g_acgt_left,t_acgt_left);
            _mm_store_pd(p_site_mixture+2,gt_left);
            
#           elif defined ( AVX_ENABLED )
            
            __m256d a = _mm256_load_pd(p_site_mixture_left);
            __m256d b = _mm256_load_pd(p_site_mixture_right);
            __m256d p = _mm_mul_pd(a,b);
            
            __m256d a_acgt = _mm256_mul_pd(p, tp_a );
            __m256d c_acgt = _mm256_mul_pd(p, tp_c );
            __m256d g_acgt = _mm256_mul_pd(p, tp_g );
            __m256d t_acgt = _mm256_mul_pd(p, tp_t );
            
            __m256d ac   = _mm256_hadd_pd(a_acgt,c_acgt);
            __m256d gt   = _mm256_hadd_pd(g_acgt,t_acgt)
            
            __m256d acgt = _mm256_hadd_pd(ac,gt);
            
            _mm256_store_pd(p_site_mixture,acgt);
            
#           else
            
            double sum_left = p_site_mixture_left[0] * tp_begin_left[0];
            sum_left += p_site_mixture_left[1] * tp_begin_left[1];
            sum_left += p_site_mixture_left[2] * tp_begin_left[2];
            sum_left += p_site_mixture_left[3] * tp_begin_left[3];
            
            double sum_right = p_site_mixture_right[0] * tp_begin_right[0];
            sum_right += p_site_mixture_right[1] * tp_begin_right[1];
            sum_right += p_site_mixture_right[2] * tp_begin_right[2];
            sum_right += p_site_mixture_right[3] * tp_begin_right[3];
            
            double sum_middle = p_site_mixture_middle[0] * tp_begin_middle[0];
            sum_middle += p_site_mixture_middle[1] * tp_begin_middle[1];
            sum_middle += p_site_mixture_middle[2] * tp_begin_middle[2];
            sum_middle += p_site_mixture_middle[3] * tp_begin_middle[3];
            
            p_site_mixture[0] = sum_left * sum_right * sum_middle;
            
            
            sum_left  = p_site_mixture_left[0] * tp_begin_left[4];
            sum_left += p_site_mixture_left[1] * tp_begin_left[5];
            sum_left += p_site_mixture_left[2] * tp_begin_left[6];
            sum_left += p_site_mixture_left[3] * tp_begin_left[7];
            
            sum_right  = p_site_mixture_right[0] * tp_begin_right[4];
            sum_right += p_site_mixture_right[1] * tp_begin_right[5];
            sum_right += p_site_mixture_right[2] * tp_begin_right[6];
            sum_right += p_site_mixture_right[3] * tp_begin_right[7];
            
            sum_middle  = p_site_mixture_middle[0] * tp_begin_middle[4];
            sum_middle += p_site_mixture_middle[1] * tp_begin_middle[5];
            sum_middle += p_site_mixture_middle[2] * tp_begin_middle[6];
            sum_middle += p_site_mixture_middle[3] * tp_begin_middle[7];
            
            p_site_mixture[1] = sum_left * sum_right * sum_middle;
            
            
            sum_left  = p_site_mixture_left[0] * tp_begin_left[8];
            sum_left += p_site_mixture_left[1] * tp_begin_left[9];
            sum_left += p_site_mixture_left[2] * tp_begin_left[10];
            sum_left += p_site_mixture_left[3] * tp_begin_left[11];
            
            sum_right  = p_site_mixture_right[0] * tp_begin_right[8];
            sum_right += p_site_mixture_right[1] * tp_begin_right[9];
            sum_right += p_site_mixture_right[2] * tp_begin_right[10];
            sum_right += p_site_mixture_right[3] * tp_begin_right[11];
            
            sum_middl  = p_site_mixture_middl[0] * tp_begin_middl[8];
            sum_middl += p_site_mixture_middl[1] * tp_begin_middl[9];
            sum_middl += p_site_mixture_middl[2] * tp_begin_middl[10];
            sum_middl += p_site_mixture_middl[3] * tp_begin_middl[11];
            
            p_site_mixture[2] = sum_left * sum_right * sum_middle;
            
            
            sum_left  = p_site_mixture_left[0] * tp_begin_left[12];
            sum_left += p_site_mixture_left[1] * tp_begin_left[13];
            sum_left += p_site_mixture_left[2] * tp_begin_left[14];
            sum_left += p_site_mixture_left[3] * tp_begin_left[15];
            
            sum_right  = p_site_mixture_right[0] * tp_begin_right[12];
            sum_right += p_site_mixture_right[1] * tp_begin_right[13];
            sum_right += p_site_mixture_right[2] * tp_begin_right[14];
            sum_right += p_site_mixture_right[3] * tp_begin_right[15];
            
            sum_middl  = p_site_mixture_middl[0] * tp_begin_middl[8];
            sum_middl += p_site_mixture_middl[1] * tp_begin_middl[9];
            sum_middl += p_site_mixture_middl[2] * tp_begin_middl[10];
            sum_middl += p_site_mixture_middl[3] * tp_begin_middl[11];
            
            p_site_mixture[3] = sum_left * sum_right * sum_middle;
            
#           endif
            
            
            // increment the pointers to the next site
            p_site_mixture_left+=this->site_offset; p_site_mixture_middle+=this->site_offset; p_site_mixture_right+=this->site_offset; p_site_mixture+=this->site_offset;
            
            
        } // end-for over all sites (=patterns)
        
    } // end-for over all mixtures (=rate-categories)
    
}


template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeRootLikelihood( size_t root, size_t left, size_t right)
{
    
    // reset the likelihood
    this->lnProb = 0.0;
    
    // get the root frequencies
    std::vector<std::vector<double> > base_frequencies_vector;
    this->getRootFrequencies(base_frequencies_vector);
    
    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = this->partial_likelihoods + this->active_likelihood[root]  *this->active_likelihood_offset + root   * this->node_offset;
    const double* p_left   = this->partial_likelihoods + this->active_likelihood[left]  *this->active_likelihood_offset + left   * this->node_offset;
    const double* p_right  = this->partial_likelihoods + this->active_likelihood[right] *this->active_likelihood_offset + right  * this->node_offset;
    
    // get pointers the likelihood for both subtrees
          double*   p_mixture          = p;
    const double*   p_mixture_left     = p_left;
    const double*   p_mixture_right    = p_right;
    
    
    bool test_underflow  = RbSettings::userSettings().getUseScaling() == true;
    bool test_this_node  = ((root+1) % RbSettings::userSettings().getScalingDensity() == 0);
    bool scale_threshold = RbSettings::userSettings().getScalingMethod() == "threshold";
    bool scale_per_mixture  = RbSettings::userSettings().getScalingPerMixture();
    
    test_underflow = test_underflow && scale_per_mixture;

    
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // get the root frequencies
        const std::vector<double> &base_frequencies = base_frequencies_vector[mixture % base_frequencies_vector.size()];

        // get pointers to the likelihood for this mixture category
              double*   p_site_mixture          = p_mixture;
        const double*   p_site_mixture_left     = p_mixture_left;
        const double*   p_site_mixture_right    = p_mixture_right;
        // iterate over all sites
        for (size_t site = 0; site < this->pattern_block_size; ++site)
        {
            
            p_site_mixture[0] = p_site_mixture_left[0] * p_site_mixture_right[0] * base_frequencies[0];
            p_site_mixture[1] = p_site_mixture_left[1] * p_site_mixture_right[1] * base_frequencies[1];
            p_site_mixture[2] = p_site_mixture_left[2] * p_site_mixture_right[2] * base_frequencies[2];
            p_site_mixture[3] = p_site_mixture_left[3] * p_site_mixture_right[3] * base_frequencies[3];
            
            if ( test_underflow == true )
            {
                if ( test_this_node == false )
                {
                    this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site];
                }
                else
                {
                    double max = ( p_site_mixture[1] > p_site_mixture[0] ? p_site_mixture[1] : p_site_mixture[0] );
                    max = ( p_site_mixture[2] > max ? p_site_mixture[2] : max );
                    max = ( p_site_mixture[3] > max ? p_site_mixture[3] : max );
                    if ( scale_threshold == false )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] - log(max);
                        p_site_mixture[0] /= max;
                        p_site_mixture[1] /= max;
                        p_site_mixture[2] /= max;
                        p_site_mixture[3] /= max;
                    }
                    else if ( max < RbConstants::SCALING_THRESHOLD )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + 1;
                        p_site_mixture[0] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[1] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[2] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[3] /= RbConstants::SCALING_THRESHOLD;
                    }
                    else
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site];
                    }
                }
            }
            
            // increment the pointers to the next site
            p_site_mixture+=this->site_offset; p_site_mixture_left+=this->site_offset; p_site_mixture_right+=this->site_offset;
            
        } // end-for over all sites (=patterns)
        
        // increment the pointers to the next mixture category
        p_mixture+=this->mixture_offset; p_mixture_left+=this->mixture_offset; p_mixture_right+=this->mixture_offset;
        
    } // end-for over all mixtures (=rate categories)
    
}

template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeRootLikelihood( size_t root, size_t left, size_t right, size_t middle)
{
    
    // reset the likelihood
    this->lnProb = 0.0;
    
    bool test_underflow     = RbSettings::userSettings().getUseScaling() == true;
    bool test_this_node     = ((root+1) % RbSettings::userSettings().getScalingDensity() == 0);
    bool scale_threshold    = RbSettings::userSettings().getScalingMethod() == "threshold";
    bool scale_per_mixture  = RbSettings::userSettings().getScalingPerMixture();

    test_underflow = test_underflow && scale_per_mixture;

    // get the root frequencies
    std::vector<std::vector<double> > base_frequencies_vector;
    this->getRootFrequencies(base_frequencies_vector);
    
    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = this->partial_likelihoods + this->active_likelihood[root]  *this->active_likelihood_offset + root   * this->node_offset;
    const double* p_left   = this->partial_likelihoods + this->active_likelihood[left]  *this->active_likelihood_offset + left   * this->node_offset;
    const double* p_right  = this->partial_likelihoods + this->active_likelihood[right] *this->active_likelihood_offset + right  * this->node_offset;
    const double* p_middle = this->partial_likelihoods + this->active_likelihood[middle]*this->active_likelihood_offset + middle * this->node_offset;
    
    // get pointers the likelihood for both subtrees
          double*   p_mixture          = p;
    const double*   p_mixture_left     = p_left;
    const double*   p_mixture_right    = p_right;
    const double*   p_mixture_middle   = p_middle;
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // get the root frequencies
        const std::vector<double> &base_frequencies = base_frequencies_vector[mixture % base_frequencies_vector.size()];

        // get pointers to the likelihood for this mixture category
              double*   p_site_mixture          = p_mixture;
        const double*   p_site_mixture_left     = p_mixture_left;
        const double*   p_site_mixture_right    = p_mixture_right;
        const double*   p_site_mixture_middle   = p_mixture_middle;
        // iterate over all sites
        for (size_t site = 0; site < this->pattern_block_size; ++site)
        {   
            p_site_mixture[0] = p_site_mixture_left[0] * p_site_mixture_right[0] * p_site_mixture_middle[0] * base_frequencies[0];
            p_site_mixture[1] = p_site_mixture_left[1] * p_site_mixture_right[1] * p_site_mixture_middle[1] * base_frequencies[1];
            p_site_mixture[2] = p_site_mixture_left[2] * p_site_mixture_right[2] * p_site_mixture_middle[2] * base_frequencies[2];
            p_site_mixture[3] = p_site_mixture_left[3] * p_site_mixture_right[3] * p_site_mixture_middle[3] * base_frequencies[3];
            
            if ( test_underflow )
            {
                if ( test_this_node == false )
                {
                    this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site];
                }
                else
                {
                    double max = ( p_site_mixture[1] > p_site_mixture[0] ? p_site_mixture[1] : p_site_mixture[0] );
                    max = ( p_site_mixture[2] > max ? p_site_mixture[2] : max );
                    max = ( p_site_mixture[3] > max ? p_site_mixture[3] : max );
                    if ( scale_threshold == false )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site] - log(max);
                        p_site_mixture[0] /= max;
                        p_site_mixture[1] /= max;
                        p_site_mixture[2] /= max;
                        p_site_mixture[3] /= max;
                    }
                    else if ( max < RbConstants::SCALING_THRESHOLD )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site] + 1;
                        p_site_mixture[0] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[1] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[2] /= RbConstants::SCALING_THRESHOLD;
                        p_site_mixture[3] /= RbConstants::SCALING_THRESHOLD;
                    }
                    else
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[root]][root][mixture][site] = this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[left]][left][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[right]][right][mixture][site] + this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[middle]][middle][mixture][site];
                    }
                }
            }
            
            // increment the pointers to the next site
            p_site_mixture+=this->site_offset; p_site_mixture_left+=this->site_offset; p_site_mixture_right+=this->site_offset; p_site_mixture_middle+=this->site_offset;
            
        } // end-for over all sites (=patterns)
        
        // increment the pointers to the next mixture category
        p_mixture+=this->mixture_offset; p_mixture_left+=this->mixture_offset; p_mixture_right+=this->mixture_offset; p_mixture_middle+=this->mixture_offset;
        
    } // end-for over all mixtures (=rate categories)
    
}


template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeRootLikelihoodNode( size_t root, size_t left, size_t right)
{
    
    // reset the likelihood
    this->lnProb = 0.0;
    
    // compute the transition probability matrix
    size_t pmat_offset_left  = this->active_pmatrices[left]  * this->active_P_matrix_offset + left  * this->pmat_node_offset;
    size_t pmat_offset_right = this->active_pmatrices[right] * this->active_P_matrix_offset + right * this->pmat_node_offset;
    
    // get the root frequencies
    std::vector<std::vector<double> > base_frequencies_vector;
    this->getRootFrequencies(base_frequencies_vector);
    
    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = this->partial_likelihoods + this->active_likelihood[root]  *this->active_likelihood_offset + root   * this->node_offset;
    const double* p_left   = this->partial_likelihoods + this->active_likelihood[left]  *this->active_likelihood_offset + left   * this->node_offset;
    const double* p_right  = this->partial_likelihoods + this->active_likelihood[right] *this->active_likelihood_offset + right  * this->node_offset;
    
    // get pointers the likelihood for both subtrees
          double*   p_mixture          = p;
    const double*   p_mixture_left     = p_left;
    const double*   p_mixture_right    = p_right;

    
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // get the root frequencies
        const std::vector<double> &base_frequencies = base_frequencies_vector[mixture % base_frequencies_vector.size()];
        
        // the transition probability matrix for this mixture category
        const double* tp_begin_left  = this->pmatrices[pmat_offset_left  + mixture].theMatrix;
        const double* tp_begin_right = this->pmatrices[pmat_offset_right + mixture].theMatrix;
        
        // get pointers to the likelihood for this mixture category
              double*   p_site_mixture          = p_mixture;
        const double*   p_site_mixture_left     = p_mixture_left;
        const double*   p_site_mixture_right    = p_mixture_right;
        // iterate over all sites
        for (size_t site = 0; site < this->pattern_block_size; ++site)
        {
            
            // get the pointers for this mixture category and this site
            const double*       tp_a_left    = tp_begin_left;
            const double*       tp_a_right   = tp_begin_right;
            
            // compute the first the likelihood of 'A'
            double  sum_left  = p_site_mixture_left[0] * tp_a_left[0];
                    sum_left += p_site_mixture_left[1] * tp_a_left[1];
                    sum_left += p_site_mixture_left[2] * tp_a_left[2];
                    sum_left += p_site_mixture_left[3] * tp_a_left[3];
            double  sum_right  = p_site_mixture_right[0] * tp_a_right[0];
                    sum_right += p_site_mixture_right[1] * tp_a_right[1];
                    sum_right += p_site_mixture_right[2] * tp_a_right[2];
                    sum_right += p_site_mixture_right[3] * tp_a_right[3];

            p_site_mixture[0] = sum_left * sum_right * base_frequencies[0];

            
            // compute the first the likelihood of 'C'
            sum_left  = p_site_mixture_left[0] * tp_a_left[4];
            sum_left += p_site_mixture_left[1] * tp_a_left[5];
            sum_left += p_site_mixture_left[2] * tp_a_left[6];
            sum_left += p_site_mixture_left[3] * tp_a_left[7];
            sum_right  = p_site_mixture_right[0] * tp_a_right[4];
            sum_right += p_site_mixture_right[1] * tp_a_right[5];
            sum_right += p_site_mixture_right[2] * tp_a_right[6];
            sum_right += p_site_mixture_right[3] * tp_a_right[7];
            
            p_site_mixture[1] = sum_left * sum_right * base_frequencies[1];
            
            
            // compute the first the likelihood of 'G'
            sum_left  = p_site_mixture_left[0] * tp_a_left[8];
            sum_left += p_site_mixture_left[1] * tp_a_left[9];
            sum_left += p_site_mixture_left[2] * tp_a_left[10];
            sum_left += p_site_mixture_left[3] * tp_a_left[11];
            sum_right  = p_site_mixture_right[0] * tp_a_right[8];
            sum_right += p_site_mixture_right[1] * tp_a_right[9];
            sum_right += p_site_mixture_right[2] * tp_a_right[10];
            sum_right += p_site_mixture_right[3] * tp_a_right[11];
            
            p_site_mixture[2] = sum_left * sum_right * base_frequencies[2];
            
            
            // compute the first the likelihood of 'T'
            sum_left  = p_site_mixture_left[0] * tp_a_left[12];
            sum_left += p_site_mixture_left[1] * tp_a_left[13];
            sum_left += p_site_mixture_left[2] * tp_a_left[14];
            sum_left += p_site_mixture_left[3] * tp_a_left[15];
            sum_right  = p_site_mixture_right[0] * tp_a_right[12];
            sum_right += p_site_mixture_right[1] * tp_a_right[13];
            sum_right += p_site_mixture_right[2] * tp_a_right[14];
            sum_right += p_site_mixture_right[3] * tp_a_right[15];
            
            p_site_mixture[3] = sum_left * sum_right * base_frequencies[3];
            
            
            
            // increment the pointers to the next site
            p_site_mixture       += this->site_offset;
            p_site_mixture_left  += this->site_offset;
            p_site_mixture_right += this->site_offset;
            
        } // end-for over all sites (=patterns)
        
        // increment the pointers to the next mixture category
        p_mixture       += this->mixture_offset;
        p_mixture_left  += this->mixture_offset;
        p_mixture_right += this->mixture_offset;
        
    } // end-for over all mixtures (=rate categories)
    
}

template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeRootLikelihoodNode( size_t root, size_t left, size_t right, size_t middle)
{
    
    // reset the likelihood
    this->lnProb = 0.0;
    
    // compute the transition probability matrix
    size_t pmat_offset_left   = this->active_pmatrices[left]   * this->active_P_matrix_offset + left   * this->pmat_node_offset;
    size_t pmat_offset_right  = this->active_pmatrices[right]  * this->active_P_matrix_offset + right  * this->pmat_node_offset;
    size_t pmat_offset_middle = this->active_pmatrices[middle] * this->active_P_matrix_offset + middle * this->pmat_node_offset;

    // get the root frequencies
    std::vector<std::vector<double> > base_frequencies_vector;
    this->getRootFrequencies(base_frequencies_vector);
    
    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = this->partial_likelihoods + this->active_likelihood[root]   * this->active_likelihood_offset + root   * this->node_offset;
    const double* p_left   = this->partial_likelihoods + this->active_likelihood[left]   * this->active_likelihood_offset + left   * this->node_offset;
    const double* p_right  = this->partial_likelihoods + this->active_likelihood[right]  * this->active_likelihood_offset + right  * this->node_offset;
    const double* p_middle = this->partial_likelihoods + this->active_likelihood[middle] * this->active_likelihood_offset + middle * this->node_offset;

    // get pointers the likelihood for both subtrees
          double*   p_mixture          = p;
    const double*   p_mixture_left     = p_left;
    const double*   p_mixture_right    = p_right;
    const double*   p_mixture_middle   = p_middle;

    
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {
        // get the root frequencies
        const std::vector<double> &base_frequencies = base_frequencies_vector[mixture % base_frequencies_vector.size()];
        
        // the transition probability matrix for this mixture category
        const double* tp_left   = this->pmatrices[pmat_offset_left   + mixture].theMatrix;
        const double* tp_right  = this->pmatrices[pmat_offset_right  + mixture].theMatrix;
        const double* tp_middle = this->pmatrices[pmat_offset_middle + mixture].theMatrix;

        // get pointers to the likelihood for this mixture category
              double*   p_site_mixture          = p_mixture;
        const double*   p_site_mixture_left     = p_mixture_left;
        const double*   p_site_mixture_right    = p_mixture_right;
        const double*   p_site_mixture_middle   = p_mixture_middle;
        // iterate over all sites
        for (size_t site = 0; site < this->pattern_block_size; ++site)
        {
            
            // compute the first the likelihood of 'A'
            double  sum_left  = p_site_mixture_left[0] * tp_left[0];
                    sum_left += p_site_mixture_left[1] * tp_left[1];
                    sum_left += p_site_mixture_left[2] * tp_left[2];
                    sum_left += p_site_mixture_left[3] * tp_left[3];
            double  sum_right  = p_site_mixture_right[0] * tp_right[0];
                    sum_right += p_site_mixture_right[1] * tp_right[1];
                    sum_right += p_site_mixture_right[2] * tp_right[2];
                    sum_right += p_site_mixture_right[3] * tp_right[3];
            double  sum_middle  = p_site_mixture_middle[0] * tp_middle[0];
                    sum_middle += p_site_mixture_middle[1] * tp_middle[1];
                    sum_middle += p_site_mixture_middle[2] * tp_middle[2];
                    sum_middle += p_site_mixture_middle[3] * tp_middle[3];

            p_site_mixture[0] = sum_left * sum_right * sum_middle * base_frequencies[0];

            
            // compute the first the likelihood of 'C'
            sum_left  = p_site_mixture_left[0] * tp_left[4];
            sum_left += p_site_mixture_left[1] * tp_left[5];
            sum_left += p_site_mixture_left[2] * tp_left[6];
            sum_left += p_site_mixture_left[3] * tp_left[7];
            sum_right  = p_site_mixture_right[0] * tp_right[4];
            sum_right += p_site_mixture_right[1] * tp_right[5];
            sum_right += p_site_mixture_right[2] * tp_right[6];
            sum_right += p_site_mixture_right[3] * tp_right[7];
            sum_middle  = p_site_mixture_middle[0] * tp_middle[4];
            sum_middle += p_site_mixture_middle[1] * tp_middle[5];
            sum_middle += p_site_mixture_middle[2] * tp_middle[6];
            sum_middle += p_site_mixture_middle[3] * tp_middle[7];
            
            p_site_mixture[1] = sum_left * sum_right * sum_middle * base_frequencies[1];
            
            
            // compute the first the likelihood of 'G'
            sum_left  = p_site_mixture_left[0] * tp_left[8];
            sum_left += p_site_mixture_left[1] * tp_left[9];
            sum_left += p_site_mixture_left[2] * tp_left[10];
            sum_left += p_site_mixture_left[3] * tp_left[11];
            sum_right  = p_site_mixture_right[0] * tp_right[8];
            sum_right += p_site_mixture_right[1] * tp_right[9];
            sum_right += p_site_mixture_right[2] * tp_right[10];
            sum_right += p_site_mixture_right[3] * tp_right[11];
            sum_middle  = p_site_mixture_middle[0] * tp_middle[8];
            sum_middle += p_site_mixture_middle[1] * tp_middle[9];
            sum_middle += p_site_mixture_middle[2] * tp_middle[10];
            sum_middle += p_site_mixture_middle[3] * tp_middle[11];
            
            p_site_mixture[2] = sum_left * sum_right * sum_middle * base_frequencies[2];
            
            
            // compute the first the likelihood of 'T'
            sum_left  = p_site_mixture_left[0] * tp_left[12];
            sum_left += p_site_mixture_left[1] * tp_left[13];
            sum_left += p_site_mixture_left[2] * tp_left[14];
            sum_left += p_site_mixture_left[3] * tp_left[15];
            sum_right  = p_site_mixture_right[0] * tp_right[12];
            sum_right += p_site_mixture_right[1] * tp_right[13];
            sum_right += p_site_mixture_right[2] * tp_right[14];
            sum_right += p_site_mixture_right[3] * tp_right[15];
            sum_middle  = p_site_mixture_middle[0] * tp_middle[12];
            sum_middle += p_site_mixture_middle[1] * tp_middle[13];
            sum_middle += p_site_mixture_middle[2] * tp_middle[14];
            sum_middle += p_site_mixture_middle[3] * tp_middle[15];
            
            p_site_mixture[3] = sum_left * sum_right * sum_middle * base_frequencies[3];
            
            
            
            // increment the pointers to the next site
            p_site_mixture        += this->site_offset;
            p_site_mixture_left   += this->site_offset;
            p_site_mixture_right  += this->site_offset;
            p_site_mixture_middle += this->site_offset;
            
        } // end-for over all sites (=patterns)
        
        // increment the pointers to the next mixture category
        p_mixture        += this->mixture_offset;
        p_mixture_left   += this->mixture_offset;
        p_mixture_right  += this->mixture_offset;
        p_mixture_middle += this->mixture_offset;
        
    } // end-for over all mixtures (=rate categories)

}


template<class charType>
void RevBayesCore::PhyloCTMCSiteHomogeneousNucleotide<charType>::computeTipLikelihood(const TopologyNode &node, size_t node_index) 
{    
    
    double* p_node = this->partial_likelihoods + this->active_likelihood[node_index]*this->active_likelihood_offset + node_index*this->node_offset;
    
    size_t data_tip_index = this->taxon_name_2_tip_index_map[ node.getName() ];
    const std::vector<bool> &gap_node = this->gap_matrix[data_tip_index];
    const std::vector<unsigned long> &char_node = this->char_matrix[data_tip_index];
    const std::vector<RbBitSet> &amb_char_node = this->ambiguous_char_matrix[data_tip_index];
    
    // compute the transition probabilities
    size_t pmat_offset = this->active_pmatrices[node_index] * this->active_P_matrix_offset + node_index * this->pmat_node_offset;
    
//    double*   p_mixture      = p_node;
    
    
    bool test_underflow  = RbSettings::userSettings().getUseScaling() == true;
    bool test_this_node  = ( (node_index+1) % RbSettings::userSettings().getScalingDensity() == 0);
    bool scale_threshold = RbSettings::userSettings().getScalingMethod() == "threshold";
    bool scale_per_mixture  = RbSettings::userSettings().getScalingPerMixture();

    test_underflow = test_underflow && scale_per_mixture;
    
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->num_site_mixtures; ++mixture)
    {

        // iterate over all sites
        for (size_t site = 0; site < this->pattern_block_size; ++site)
        {
        
            
            // the transition probability matrix for this mixture category
            const double*       tp_begin    = this->pmatrices[pmat_offset + mixture].theMatrix;
        
            // get the pointer to the likelihoods for this site and mixture category
            size_t offset = mixture*this->mixture_offset + site*this->site_offset;

            double* p_site_mixture      = p_node + offset;
//            double* p_site_mixture      = p_mixture;
            
            // is this site a gap?
            if ( gap_node[site] ) 
            {
                // since this is a gap we need to assume that the actual state could have been any state
                p_site_mixture[0] = 1.0;
                p_site_mixture[1] = 1.0;
                p_site_mixture[2] = 1.0;
                p_site_mixture[3] = 1.0;
                
                if ( test_underflow )
                {
                    this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = 0;
                }
            } 
            else // we have observed a character
            {
                                    
                if ( this->using_ambiguous_characters == true )
                {
                    // get the original character
                    const RbBitSet &org_val = amb_char_node[site];
                    
                    double p0 = 0.0;
                    double p1 = 0.0;
                    double p2 = 0.0;
                    double p3 = 0.0;
                    
                    if ( org_val.test(0) == true )
                    {
                        p0 = tp_begin[0];
                        p1 = tp_begin[4];
                        p2 = tp_begin[8];
                        p3 = tp_begin[12];
                    }
                    
                    if ( org_val.test(1) == true )
                    {
                        p0 += tp_begin[1];
                        p1 += tp_begin[5];
                        p2 += tp_begin[9];
                        p3 += tp_begin[13];
                    }
                    
                    if ( org_val.test(2) == true )
                    {
                        p0 += tp_begin[2];
                        p1 += tp_begin[6];
                        p2 += tp_begin[10];
                        p3 += tp_begin[14];
                    }
                    
                    if ( org_val.test(3) == true )
                    {
                        p0 += tp_begin[3];
                        p1 += tp_begin[7];
                        p2 += tp_begin[11];
                        p3 += tp_begin[15];
                    }
                    
                    p_site_mixture[0] = p0;
                    p_site_mixture[1] = p1;
                    p_site_mixture[2] = p2;
                    p_site_mixture[3] = p3;
                    
                } 
                else // no ambiguous characters in use
                {
                    
                    // get the original character
                    unsigned long org_val = char_node[site];
                    
                    // store the likelihood
                    p_site_mixture[0] = tp_begin[org_val];
                    p_site_mixture[1] = tp_begin[4+org_val];
                    p_site_mixture[2] = tp_begin[8+org_val];
                    p_site_mixture[3] = tp_begin[12+org_val];
                        
                }
                
                
                if ( test_underflow )
                {
                    if (test_this_node == false )
                    {
                        this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = 0;
                    }
                    else
                    {
                        double max = ( p_site_mixture[1] > p_site_mixture[0] ? p_site_mixture[1] : p_site_mixture[0] );
                        max = ( p_site_mixture[2] > max ? p_site_mixture[2] : max );
                        max = ( p_site_mixture[3] > max ? p_site_mixture[3] : max );
                        if ( scale_threshold == false )
                        {
                            this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = - log(max);
                            p_site_mixture[0] /= max;
                            p_site_mixture[1] /= max;
                            p_site_mixture[2] /= max;
                            p_site_mixture[3] /= max;
                        }
                        else if ( max < RbConstants::SCALING_THRESHOLD )
                        {
                            this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = 1;
                            p_site_mixture[0] /= RbConstants::SCALING_THRESHOLD;
                            p_site_mixture[1] /= RbConstants::SCALING_THRESHOLD;
                            p_site_mixture[2] /= RbConstants::SCALING_THRESHOLD;
                            p_site_mixture[3] /= RbConstants::SCALING_THRESHOLD;
                        }
                        else
                        {
                            this->per_node_site_mixture_log_scaling_factors[this->active_likelihood[node_index]][node_index][mixture][site] = 0;
                        }
                    }
                }
                                
                
            } // end-if a gap state
            
            
            // increment the pointers to next site
//            p_site_mixture+=this->site_offset;
            
        } // end-for over all sites/patterns in the sequence

        
        // increment the pointers to next mixture category
//        p_mixture+=this->mixture_offset;
        
    } // end-for over all mixture categories

}


#endif
