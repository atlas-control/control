#!/usr/bin/env python
#
# $Id: ut_xaodrootaccess_transtree2_test.py 653633 2015-03-12 13:44:48Z krasznaa $
#
# Unit test for the transient tree creating infrastructure
#

## C/C++ style main function
def main():

    # The name of the application:
    APP_NAME = "ut_xaodrootaccess_transtree2_test"

    # Set up a logger object:
    import logging
    logger = logging.getLogger( APP_NAME )
    logger.setLevel( logging.INFO )
    hdlr = logging.StreamHandler( sys.stdout )
    frmt = logging.Formatter( "%(name)-14s%(levelname)8s %(message)s" )
    hdlr.setFormatter( frmt )
    logger.addHandler( hdlr )

    # Set up the environment:
    import ROOT
    if ROOT.gROOT.Macro( "$ROOTCOREDIR/scripts/load_packages.C" ):
        logger.error( "Couldn't load the RootCore packages" )
        return 1
    if ROOT.xAOD.Init( APP_NAME ).isFailure():
        logger.error( "Failed to call xAOD::Init(...)" )
        return 1

    # Set up a TChain as input:
    chain = ROOT.TChain( "CollectionTree" )
    chain.Add( "/afs/cern.ch/atlas/project/PAT/xAODs/r5787/"
               "mc14_13TeV.110401.PowhegPythia_P2012_ttbar_nonallhad.merge.AOD."
               "e2928_s1982_s2008_r5787_r5853_tid01597980_00/"
               "AOD.01597980._000098.pool.root.1" )
    chain.Add( "/afs/cern.ch/atlas/project/PAT/xAODs/r5787/"
               "mc14_13TeV.110401.PowhegPythia_P2012_ttbar_nonallhad.merge.AOD."
               "e2928_s1982_s2008_r5787_r5853_tid01597980_00/"
               "AOD.01597980._000420.pool.root.1" );

    # Create a transient tree:
    tree = ROOT.xAOD.MakeTransientTree( chain )
    if not tree:
        logger.error( "Couldn't create transient tree from TChain input" )
        return 1

    # Fix up the DataVector iterators for PyROOT:
    import xAODRootAccess.GenerateDVIterators

    # Loop over a few event:
    for entry in xrange( 10 ):
        # Load the event:
        if tree.GetEntry( entry ) < 0:
            logger.error( "Couldn't load entry %i" % entry )
            return 1
        logger.info( "Processing entry %i" % entry )
        # Print the properties of the electrons:
        logger.info( "  Number of electrons: %i" % \
                      tree.ElectronCollection.size() )
        for el in tree.ElectronCollection:
            logger.info( "   - eta: %g, phi: %g, pt: %g" % \
                         ( el.eta(), el.phi(), el.pt() ) )
            pass
        pass

    # Return gracefully:
    return 0

# Run the main() function:
if __name__ == "__main__":
    import sys
    sys.exit( main() )
