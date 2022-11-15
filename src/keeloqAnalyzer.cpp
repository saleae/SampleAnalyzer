#include "keeloqAnalyzer.h"
#include "keeloqAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <iostream>
using namespace std;

keeloqAnalyzer::keeloqAnalyzer()
:	Analyzer2(),  
	mSettings( new keeloqAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );

	UseFrameV2();
}

keeloqAnalyzer::~keeloqAnalyzer()
{
	KillThread();
}

void keeloqAnalyzer::SetupResults()
{
	mResults.reset( new keeloqAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void keeloqAnalyzer::WorkerThread()
{
    char result_str[ 128 ];

	mKeeloq = GetAnalyzerChannelData( mSettings->mInputChannel );

    mResults->AddResultString( "Hello test\n" );

    if( mKeeloq->GetBitState() == BIT_HIGH )
        mKeeloq->AdvanceToNextEdge();

	for( ; ; )
	{
        bool bad_pckt = false;
		U64 data = 0;
        U64 data1 = 0;
        U64 width = mKeeloq->GetSampleOfNextEdge();
        mSampleW_preamb.clear();

        //looking a preambule
        
        while( width < 20000 )
        {
            mKeeloq->AdvanceToNextEdge();
            width = mKeeloq->GetSampleOfNextEdge();
        }
        
        mResults->AddMarker( mKeeloq->GetSampleNumber(), AnalyzerResults::Start, mSettings->mInputChannel );

		U64 starting_sample = mKeeloq->GetSampleNumber();
        
        mResults->AddResultString( "preambule" );
		for( U32 i=0; i<24-1; i++ ) //through preambule
		{
            mKeeloq->AdvanceToNextEdge();
            mSampleW_preamb.push_back( mKeeloq->GetSampleOfNextEdge() - mKeeloq->GetSampleNumber() );
		}
        mKeeloq->AdvanceToNextEdge();

		for( int i = 0; i < mSampleW_preamb.size() - 1; i++)
        {
            U64 curr_w = abs(int(mSampleW_preamb[ i ] - mSampleW_preamb[ i + 1 ]));
            if( curr_w > 200 )
                bad_pckt = true;
        }

        // looking a Fixed Portion 34 bits
        mResults->AddMarker( mKeeloq->GetSampleNumber(), AnalyzerResults::X, mSettings->mInputChannel );
        mResults->AddResultString( "Fixed portion" );
		
        for( U32 i = 0; i < 34; i++ ) 
        {
            mKeeloq->AdvanceToNextEdge();
            mKeeloq->Advance( mSampleW_preamb[1] * 1.5);    //set to middle
            if( mKeeloq->GetBitState() == BIT_HIGH )
            {   //0
                
                mResults->AddMarker( mKeeloq->GetSampleNumber(), AnalyzerResults::Square, mSettings->mInputChannel );
                mKeeloq->AdvanceToNextEdge();            
            }
            else
            {
                // 1
                data |= 1;
                mResults->AddMarker( mKeeloq->GetSampleNumber(), AnalyzerResults::Square, mSettings->mInputChannel );
            }
            data <<= 1;
        }

		// looking a Encrypted Portion 32 bits
        mResults->AddResultString( "Encrupted portion" );
        for( U32 i = 0; i < 32; i++ ) 
        {
            mKeeloq->AdvanceToNextEdge();
            mKeeloq->Advance( mSampleW_preamb[ 1 ] * 1.5 ); // set to middle
            if( mKeeloq->GetBitState() == BIT_HIGH )
            {   //0
                mResults->AddMarker( mKeeloq->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
                mKeeloq->AdvanceToNextEdge();
            }
            else
            {
                // 1
                data1 |= 1;
                mResults->AddMarker( mKeeloq->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
            }
            data1 <<= 1;
        }
        mKeeloq->Advance( mSampleW_preamb[ 1 ] * 1.5 ); // set to middle
        mResults->AddMarker( mKeeloq->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mInputChannel );

        if( !bad_pckt )
        {
            // we have a byte to save.
            Frame frame;
            frame.mData1 = data;
            frame.mData2 = data1;
            frame.mFlags = 0;
            frame.mStartingSampleInclusive = starting_sample;
            frame.mEndingSampleInclusive = mKeeloq->GetSampleNumber();

            mResults->AddFrame( frame );

            // New FrameV2 code.
            FrameV2 frame_v2;
            // you can add any number of key value pairs. Each will get it's own column in the data table.
            frame_v2.AddInteger( "Fixed portion", frame.mData1 );
            frame_v2.AddInteger( "Encrupted portion", frame.mData2 );

            // This actually saves your new FrameV2. In this example, we just copy the same start and end sample number from Frame V1 above.
            // The second parameter is the frame "type". Any string is allowed.
            mResults->AddFrameV2( frame_v2, "packet", starting_sample, frame.mEndingSampleInclusive );

            // You should already be calling this to make submitted frames available to the rest of the system. It's still required.

            mResults->CommitResults();
            ReportProgress( frame.mEndingSampleInclusive );
        }
	}
}

bool keeloqAnalyzer::NeedsRerun()
{
	return false;
}

U32 keeloqAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 keeloqAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* keeloqAnalyzer::GetAnalyzerName() const
{
	return "KEELOQ Code Hopping Encoder";
}

const char* GetAnalyzerName()
{
	return "KEELOQ Code Hopping Encoder";
}

Analyzer* CreateAnalyzer()
{
	return new keeloqAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}