#include "ExponentialDemographicFunction.h"

#include <cmath>

#include "Cloneable.h"
#include "RbException.h"
#include "TypedDagNode.h"

namespace RevBayesCore { class DagNode; }

using namespace RevBayesCore;

/**
 * @param[in]    N0   Pointer to the population size at the beginning of the exponential period (towards the present)
 * @param[in]    N1   Pointer to the population size at the end of the exponential period (towards the past)
 * @param[in]    t0   Pointer to the time at which the exponential process started
 * @param[in]    t1   Pointer to the time of the beginning of the exponential period (towards the present)
 */
ExponentialDemographicFunction::ExponentialDemographicFunction(const TypedDagNode<double>* N0, const TypedDagNode<double>* N1, const TypedDagNode<double>* t0, const TypedDagNode<double>* t1) : DemographicFunction(),
    theta_ancient( N1 ),
    theta_recent( N0 ),
    time_ancient( t1 ),
    time_recent( t0 )
{
    addVariable( N0 );
    addVariable( N1 );
    addVariable( t0 );
    addVariable( t1 );
}

/**
 * @param[in]    f    The exponential demographic function to copy.
 */
ExponentialDemographicFunction::ExponentialDemographicFunction(const ExponentialDemographicFunction &f) : DemographicFunction(f),
    theta_ancient( f.theta_ancient ),
    theta_recent( f.theta_recent ),
    time_ancient( f.time_ancient ),
    time_recent( f.time_recent )
{
    
}


ExponentialDemographicFunction::~ExponentialDemographicFunction( void )
{
    
}

/**
 * @param[in]    f    The exponential demographic function to copy.
 */
ExponentialDemographicFunction& ExponentialDemographicFunction::operator=(const ExponentialDemographicFunction &f)
{
    DemographicFunction::operator=( f );
    
    if ( this != &f )
    {
        theta_ancient   = f.theta_ancient;
        theta_recent    = f.theta_recent;
        time_ancient    = f.time_ancient;
        time_recent     = f.time_recent;
    }
    
    return *this;
}

/**
 * This is similar to the copy constructor but useful in inheritance.
 *
 * The clone function is a convenience function to create proper copies of inherited objects.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'B'.
 *
 * @return A new copy of myself
 */
ExponentialDemographicFunction* ExponentialDemographicFunction::clone( void ) const
{
    
    return new ExponentialDemographicFunction(*this);
}

/**
 * @param[in]   t    Time
 *
 * @return  N(t)
 */
double ExponentialDemographicFunction::getDemographic(double t) const
{
    double N0 = theta_recent->getValue();
    double N1 = theta_ancient->getValue();
    double t0 = time_recent->getValue();
    double t1 = time_ancient->getValue();
    
    if ( t1 < t0 || t0 < 0 || N1 < 0 || t < t0 || t > t1 )
    {
        throw RbException("Impossible parameter values in exponential growth/decline demographic functions.");
    }
    
    if ( N0 == N1 )
    {
        return N0;
    }
    else
    {
        double alpha = log( N1/N0 ) / (t0 - t1);
        return N0 * exp( (t0-t) * alpha);
    }
    
}

/**
 * @param[in]   start   Time at which the interval starts
 * @param[in]   finish  Time at which the interval ends
 *
 * @return  Integral 1/N(x) dx between start and finish.
 */
double ExponentialDemographicFunction::getIntegral(double start, double finish) const
{
    double N0 = theta_recent->getValue();
    double N1 = theta_ancient->getValue();
    double t0 = time_recent->getValue();
    double t1 = time_ancient->getValue();
    
    if ( t1 < t0 || t0 < 0 || N1 < 0 || start < t0 || start > t1 || finish < t0 || finish > t1 )
    {
        throw RbException("Impossible parameter values in exponential growth/decline demographic functions.");
    }
    
    
    if ( N0 == N1 )
    {
        double delta = finish - start;
        return delta / N0;
    }
    else
    {
        double alpha = log( N1/N0 ) / (t0 - t1);
        return (exp( (finish-t0)*alpha ) - exp((start-t0)*alpha)) / N0 / alpha;
    }
    
}

/**
 * spaceholder
 */
double ExponentialDemographicFunction::getWaitingTime(double time, double lambda) const
{
    double N0 = theta_recent->getValue();
    double N1 = theta_ancient->getValue();
    double t0 = time_recent->getValue();
    double t1 = time_ancient->getValue();
    
	double alpha = log( N1/N0 ) / (t0 - t1);
	double inlog = exp(alpha * (time - t0)) + lambda * alpha * theta_recent->getValue();
	if (inlog < 0)
	{
	    return -1;
	}
	else 
	{
	    return log(inlog) / alpha - time;
	}
}

/**
 * @param[in]   old_node    Pointer to the DAG node to be replaced
 * @param[in]   new_node    Pointer to the DAG node replacing the other
 */
void ExponentialDemographicFunction::swapNodeInternal(const DagNode *old_node, const DagNode *new_node)
{
    
    if (old_node == theta_ancient)
    {
        theta_ancient = static_cast<const TypedDagNode<double>* >( new_node );
    }
    
    if (old_node == theta_recent)
    {
        theta_recent = static_cast<const TypedDagNode<double>* >( new_node );
    }
    
    if (old_node == time_ancient)
    {
        time_ancient = static_cast<const TypedDagNode<double>* >( new_node );
    }
    
    if (old_node == time_recent)
    {
        time_recent = static_cast<const TypedDagNode<double>* >( new_node );
    }
    
}


std::ostream& operator<<(std::ostream& o, const ExponentialDemographicFunction& x)
{
    return o;
}
