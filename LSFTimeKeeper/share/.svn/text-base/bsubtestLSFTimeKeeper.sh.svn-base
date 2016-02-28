#this is meant to be just an example to modify according to your paths/needs


# some variables that might be used by atlsim
ATLAS_ROOT=/afs/cern.ch/atlas
ATLAS=$ATLAS_ROOT/software
BASE=$ATLAS_ROOT/software/dist
DATA=$ATLAS_ROOT/offline/data
export ATLAS_ROOT ATLAS BASE DATA


#BSUB -J paolo_test

# Decide on time needed
#BSUB -q 8nm
##### -q 1nh

# Give name of log-file
#BSUB -o paolo_test.log


# setup cmt
cd /afs/cern.ch/user/c/calaf/scratch0/to750
. /afs/cern.ch/user/c/calaf/.cmt/drq_setup.sh atlrel_5
CMTPATH=`pwd`:$CMTPATH
export CMTPATH
echo $CMTPATH

cd Control/LSFTimeKeeper/LSFTimeKeeper-00-00-00/run
pwd
. ../cmt/setup.sh
time ../$CMTCONFIG/LSFTimeKeeperSvc_test.exe

exit
