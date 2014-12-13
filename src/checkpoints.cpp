// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
     static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (     0, uint256("0x0000f31e7eed3db329dddb977cdad26e68f6da4b4f02834ecc49799a0b98d5e0"))
        (  5555, uint256("0x0000000000d4304ed8edf5659c1e1e6f69860d4c94263d079469bc534f3d023b"))
        (  9999, uint256("0x00000000001c1f7dc6e332dd9c6b3444eaa1aee6459c11a41fa3becc63cfdfaa"))
        ( 17031, uint256("0x00000000004dce12446a3860e8cb7f4e45fcd105da3c5e00826cc71b4d3b1dc3"))
        ( 33560, uint256("0x000000000094f0d6ffcb76f1e971cf4a0ecd4dfe04f67abc7502f36ab036f0a6"))
        ( 70001, uint256("0x0000000000257347e4736b1b15537e613cf0b42905c87293f133fab1c7431fc3"))
        ( 82111, uint256("0x00000000001c5989284e076e1a7a8e9aac8b423b4bc723637207f288130ea14b"))
        ( 93001, uint256("0x00000000002e692e30b5c9c3bfe13c4461bcf78abc49eb423f8023ec992b1ddb"))
        (103333, uint256("0x00000000006ab21ed3dc6d4bb8bb0c1127334d587a7dda8743516ddf29ffc8ed"))
        (111111, uint256("0x0000000000158bc9663fbbe97b4722468bebbec3fbf97a7f4dc21ca6b85adf2c"))
        (120001, uint256("0x00000000001f765535daacce7296f845513b19e011243a00b60c0a9c80582c4c"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1407218400, // * UNIX timestamp of last checkpoint block
        1,     // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        5000      // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        (0, uint256("0x0000d08a1f00c71f66e4da6c33f68c435afcc86f2c3a1915161fc51a24d03d8d"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1403845505,
        1,
        300
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
