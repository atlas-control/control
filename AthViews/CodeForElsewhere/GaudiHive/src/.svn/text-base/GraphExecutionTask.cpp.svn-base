// Framework
#include "GraphExecutionTask.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ContextSpecificPtr.h"

// C++
#include <functional>
#include "tbb/flow_graph.h"

// local includes
#include "RetCodeGuard.h"

#include<iostream>

//The method for scheduling a subgraph
tbb::task* GraphExecutionTask::execute()
{
	if ( !m_algoIndices.size() ) return nullptr;

	tbb::flow::graph g;
	std::vector< tbb::flow::continue_node< tbb::flow::continue_msg >* > allNodes;
	for ( unsigned int algIndex = 0; algIndex < m_algoIndices.size(); ++algIndex )
	{
		//Create a node for each algorithm
		allNodes.push_back( new tbb::flow::continue_node< tbb::flow::continue_msg>
				( g, [&,algIndex]( const tbb::flow::continue_msg & )
				  {
				    Algorithm * algoPtr = dynamic_cast< Algorithm* >( m_algorithms[algIndex] );

				    //Retrieve old context
				    auto oldContext = algoPtr->getContext();

				    //Run algorithm in context provided
				    algoPtr->setContext( m_eventContext );
				    algoPtr->sysExecute();

				    //Return the algorithm context to what it was before
				    algoPtr->setContext( oldContext );
				  }
				) );

		//Connect 2nd node to 1st, and so on
		if ( algIndex )
		{
			tbb::flow::make_edge( *allNodes[ algIndex - 1 ], *allNodes[ algIndex ] );
		}
	}

	allNodes[0]->try_put( tbb::flow::continue_msg() );
	g.wait_for_all();
	return nullptr;
}
