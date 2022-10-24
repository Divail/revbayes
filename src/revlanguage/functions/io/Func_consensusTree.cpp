#include <stddef.h>
#include <sstream>
#include <string>
#include <vector>

#include "ArgumentRule.h"
#include "Func_consensusTree.h"
#include "NexusWriter.h"
#include "RlBranchLengthTree.h"
#include "RlString.h"
#include "RlTimeTree.h"
#include "RlTraceTree.h"
#include "Probability.h"
#include "Argument.h"
#include "ArgumentRules.h"
#include "Clade.h"
#include "RbBoolean.h"
#include "RevPtr.h"
#include "RevVariable.h"
#include "RlBoolean.h"
#include "RlFunction.h"
#include "RlTree.h"
#include "Taxon.h"
#include "TopologyNode.h"
#include "TraceTree.h"
#include "Tree.h"
#include "TreeSummary.h"
#include "TypeSpec.h"
#include "WorkspaceVector.h"

using namespace RevLanguage;


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Func_consensusTree* Func_consensusTree::clone(void) const
{
    
    return new Func_consensusTree(*this);
}


/** Execute function */
RevPtr<RevVariable> Func_consensusTree::execute(void)
{
    size_t arg_index = 0;

    RevBayesCore::TreeSummary* summary = NULL;

    RevObject& b = args[arg_index++].getVariable()->getRevObject();
    if ( b.isType( TraceTree::getClassTypeSpec() ) )
    {
        summary = &( static_cast<TraceTree &>(b).getValue().summary() );
    }
    else
    {
        std::vector<TraceTree>& tt = static_cast<const WorkspaceVector<TraceTree> &>(b).getValue();

        std::vector<RevBayesCore::TraceTree* > vec;
        for(size_t i = 0; i < tt.size(); i++)
        {
            vec.push_back(&(tt[i].getValue()));
        }

        summary = new RevBayesCore::TreeSummary( vec, tt.front().getValue().isClock() );
    }
    
    double cutoff = static_cast<const Probability &>(args[arg_index++].getVariable()->getRevObject()).getValue();

    const std::string& filename = static_cast<const RlString&>( args[arg_index++].getVariable()->getRevObject() ).getValue();


    RevBayesCore::TreeSummary::AnnotationReport report;

    report.node_ages_HPD                 = static_cast<const Probability &>(args[arg_index++].getVariable()->getRevObject()).getValue();
    report.mean_node_ages                = static_cast<const RlBoolean &>( this->args[arg_index++].getVariable()->getRevObject() ).getValue();
    report.sampled_ancestor_probs        = static_cast<const RlBoolean &>( this->args[arg_index++].getVariable()->getRevObject() ).getValue();
    report.force_positive_branch_lengths = static_cast<const RlBoolean &>( this->args[arg_index++].getVariable()->getRevObject() ).getValue();

    bool verbose = true;
    RevBayesCore::Tree* tree = summary->mrTree(report, cutoff, verbose);
    
    if ( filename != "" )
    {
        
        RevBayesCore::NexusWriter writer(filename);
        writer.openStream(false);
        
        std::vector<RevBayesCore::Taxon> taxa;
        tree->getRoot().getTaxa(taxa);
        RevBayesCore::Clade c( taxa );
        writer.writeNexusBlock(c);
        
        writer.writeNexusBlock(*tree);
        
        writer.closeStream();
        
    }
    
    Tree* t;
    if ( summary->isClock() )
    {
        t = new TimeTree( tree );
    }
    else
    {
        t = new BranchLengthTree( tree );
    }

    if ( b.isType( TraceTree::getClassTypeSpec() ) == false )
    {
        delete summary;
    }

    return new RevVariable( t );
}



/** Get argument rules */
const ArgumentRules& Func_consensusTree::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool rules_set = false;
    
    if (!rules_set)
    {
        
        std::vector<TypeSpec> traceArgRules;
        traceArgRules.push_back( TraceTree::getClassTypeSpec() );
        traceArgRules.push_back( WorkspaceVector<TraceTree>::getClassTypeSpec() );
        argumentRules.push_back( new ArgumentRule( "trace", traceArgRules, "The samples of trees from the posterior.", ArgumentRule::BY_REFERENCE, ArgumentRule::ANY ) );
        argumentRules.push_back( new ArgumentRule( "cutoff"   , Probability::getClassTypeSpec()  , "The minimum threshold for clade probabilities.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new Probability(0.5) ) );
        argumentRules.push_back( new ArgumentRule( "file"     , RlString::getClassTypeSpec() , "The name of the file for storing the tree.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlString("") ) );
        
        argumentRules.push_back( new ArgumentRule( "hpd"   ,    Probability::getClassTypeSpec() , "The probability mass of the highest posterior density node age interval.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new Probability(0.95) ) );
        argumentRules.push_back( new ArgumentRule( "mean" , RlBoolean::getClassTypeSpec() , "Annotate node ages using the mean age instead of the median?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );
        argumentRules.push_back( new ArgumentRule( "sampledAncestors" , RlBoolean::getClassTypeSpec() , "Annotate sampled ancestor probs?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(true) ) );
        argumentRules.push_back( new ArgumentRule( "positiveBranchLengths" , RlBoolean::getClassTypeSpec() , "Force negative branch lengths to be short but positive?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );

        rules_set = true;
    }
    
    return argumentRules;
}


/** Get Rev type of object */
const std::string& Func_consensusTree::getClassType(void)
{
    
    static std::string rev_type = "Func_consensusTree";
    
    return rev_type;
}


/** Get class type spec describing type of object */
const TypeSpec& Func_consensusTree::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_consensusTree::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "consensusTree";
    
    return f_name;
}

/**
 * Get the name for this procedure.
 */
std::vector<std::string> Func_consensusTree::getFunctionNameAliases( void ) const
{
    std::vector<std::string> aliases;

    aliases.push_back("conTree");
    aliases.push_back("sumt");

    return aliases;
}


/** Get type spec */
const TypeSpec& Func_consensusTree::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Get return type */
const TypeSpec& Func_consensusTree::getReturnType( void ) const
{
    
    static TypeSpec return_typeSpec = Tree::getClassTypeSpec();
    
    return return_typeSpec;
}

