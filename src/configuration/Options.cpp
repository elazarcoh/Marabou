/*********************                                                        */
/*! \file Options.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Guy Katz
 ** This file is part of the Marabou project.
 ** Copyright (c) 2017-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved. See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** [[ Add lengthier description here ]]

**/

#include "ConfigurationError.h"
#include "Debug.h"
#include "GlobalConfiguration.h"
#include "Options.h"
#include "PiecewiseLinearCaseSplit.h"

Options *Options::get()
{
    static Options singleton;
    return &singleton;
}

Options::Options()
    : _optionParser( &_boolOptions, &_intOptions, &_floatOptions, &_stringOptions )
{
    initializeDefaultValues();
    _optionParser.initialize();
}

Options::Options( const Options & )
{
    // This constructor should never be called
    ASSERT( false );
}

void Options::initializeDefaultValues()
{
    /*
      Bool options
    */
    _boolOptions[DNC_MODE] = false;
    _boolOptions[PREPROCESSOR_PL_CONSTRAINTS_ADD_AUX_EQUATIONS] = false;
    _boolOptions[RESTORE_TREE_STATES] = false;
    _boolOptions[ITERATIVE_PROPAGATION] = false;
    _boolOptions[SOLVE_WITH_MILP] = false;

    /*
      Int options
    */
    _intOptions[NUM_WORKERS] = 1;
    _intOptions[NUM_INITIAL_DIVIDES] = 0;
    _intOptions[NUM_ONLINE_DIVIDES] = 2;
    _intOptions[INITIAL_TIMEOUT] = 5;
    _intOptions[VERBOSITY] = 2;
    _intOptions[TIMEOUT] = 0;
    _intOptions[CONSTRAINT_VIOLATION_THRESHOLD] = 20;

    /*
      Float options
    */
    _floatOptions[TIMEOUT_FACTOR] = 1.5;
    _floatOptions[MILP_SOLVER_TIMEOUT] = 1.0;
    _floatOptions[PREPROCESSOR_BOUND_TOLERANCE] = \
        GlobalConfiguration::DEFAULT_EPSILON_FOR_COMPARISONS;

    /*
      String options
    */
    _stringOptions[INPUT_FILE_PATH] = "";
    _stringOptions[PROPERTY_FILE_PATH] = "";
    _stringOptions[INPUT_QUERY_FILE_PATH] = "";
    _stringOptions[SUMMARY_FILE] = "";
    _stringOptions[SPLITTING_STRATEGY] = "";
    _stringOptions[SNC_SPLITTING_STRATEGY] = "";
    _stringOptions[QUERY_DUMP_FILE] = "";

    /*
      GammaAbstract options
    */
    _gammaAbstractOption[GAMMA_ABSTRACT] = Map< unsigned, Pair<unsigned, unsigned> >();
    /*
      VarIndexToPos options
    */
    _varIndexToPosOption[VAR_INDEX_TO_POS] = Map< unsigned, bool >();
    /*
      VarIndexToInc options
    */
    _varIndexToIncOption[VAR_INDEX_TO_INC] = Map< unsigned, bool >();
    /*
      PostVarIndices options
    */
    _postVarIndicesOption[POST_VAR_INDICES] = Map< unsigned, unsigned >();
    /*
      GammaAbstract options
    */
    _gammaOption[GAMMA] = List<Map<unsigned, bool>>();

}

void Options::parseOptions( int argc, char **argv )
{
    _optionParser.parse( argc, argv );
}

void Options::printHelpMessage() const
{
    _optionParser.printHelpMessage();
}

bool Options::getBool( unsigned option ) const
{
    return _boolOptions.get( option );
}

int Options::getInt( unsigned option ) const
{
    return _intOptions.get( option );
}

float Options::getFloat( unsigned option ) const
{
    return _floatOptions.get( option );
}

String Options::getString( unsigned option ) const
{
    return String( _stringOptions.get( option ) );
}

Map<unsigned, Pair<unsigned, unsigned>> Options::getGammaAbstract( unsigned option ) const
{
    return _gammaAbstractOption.get( option );
}

List<Map<unsigned, bool>> Options::getGamma( unsigned option ) const
{
    return _gammaOption.get( option );
}

Map<unsigned, bool> Options::getVarIndexToPos( unsigned option ) const
{
    return _varIndexToPosOption.get( option );
}

Map<unsigned, bool> Options::getVarIndexToInc( unsigned option ) const
{
    return _varIndexToIncOption.get( option );
}

Map<unsigned, unsigned> Options::getPostVarIndices( unsigned option ) const
{
    return _postVarIndicesOption.get( option );
}

void Options::setBool( unsigned option, bool value )
{
    _boolOptions[option] = value;
}

void Options::setInt( unsigned option, int value )
{
    _intOptions[option] = value;
}

void Options::setFloat( unsigned option, float value )
{
    _floatOptions[option] = value;
}

void Options::setString( unsigned option, std::string value )
{
    _stringOptions[option] = value;
}

void Options::setGammaAbstract( unsigned option,
                            Map<unsigned, Pair<unsigned, unsigned>> values )
{
    _gammaAbstractOption[option] = Map< unsigned, Pair<unsigned, unsigned> >();
    for (auto item: values){
        _gammaAbstractOption[option].insert(item.first, item.second);
    }
}

void Options::setGamma( unsigned option,
                        List<Map<unsigned, bool>> values )
{
    _gammaOption[option] = List<Map<unsigned, bool>>();
    for (auto item: values){
        _gammaOption[option].append(item);
    }
}

void Options::setVarIndexToPos( unsigned option, Map<unsigned, bool> values )
{
    for (auto item: values){
        _varIndexToPosOption[option].insert(item.first, item.second);
    }
}

void Options::setVarIndexToInc( unsigned option, Map<unsigned, bool> values )
{
    for (auto item: values){
        _varIndexToIncOption[option].insert(item.first, item.second);
    }
}

void Options::setPostVarIndices( unsigned option, Map<unsigned, unsigned> values )
{
    for (auto item: values){
        _postVarIndicesOption[option].insert(item.first, item.second);
    }
}

DivideStrategy Options::getDivideStrategy() const
{
    String strategyString = String( _stringOptions.get
                                    ( Options::SPLITTING_STRATEGY ) );
    if ( strategyString == "polarity" )
        return DivideStrategy::Polarity;
    if ( strategyString == "earliest-relu" )
        return DivideStrategy::EarliestReLU;
    if ( strategyString == "relu-violation" )
        return DivideStrategy::ReLUViolation;
    else if ( strategyString == "largest-interval" )
        return DivideStrategy::LargestInterval;
    else
        return DivideStrategy::Auto;
}

SnCDivideStrategy Options::getSnCDivideStrategy() const
{
    String strategyString = String( _stringOptions.get( Options::SNC_SPLITTING_STRATEGY ) );
    if ( strategyString == "polarity" )
        return SnCDivideStrategy::Polarity;
    else if ( strategyString == "largest-interval" )
        return SnCDivideStrategy::LargestInterval;
    else
        return SnCDivideStrategy::Auto;
}

MILPSolverBoundTighteningType Options::getMILPSolverBoundTighteningType() const
{
    if ( gurobiEnabled() )
    {
        String strategyString = String( _stringOptions.get( Options::MILP_SOLVER_BOUND_TIGHTENING_TYPE ) );
        if ( strategyString == "lp" )
            return LP_RELAXATION;
        else if ( strategyString == "lp-inc" )
            return LP_RELAXATION_INCREMENTAL;
        else if ( strategyString == "milp" )
            return MILP_ENCODING;
        else if ( strategyString == "milp-inc" )
            return MILP_ENCODING_INCREMENTAL;
        else if ( strategyString == "iter-prop" )
            return ITERATIVE_PROPAGATION;
        else if ( strategyString == "none" )
            return NONE;
        else
            return LP_RELAXATION;
    }
    else
    {
        return NONE;
    }
}

//
// Local Variables:
// compile-command: "make -C ../.. "
// tags-file-name: "../../TAGS"
// c-basic-offset: 4
// End:
//
