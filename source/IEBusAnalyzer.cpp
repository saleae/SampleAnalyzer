#include "IEBusAnalyzer.h"
#include "IEBusAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

IEBusAnalyzer::IEBusAnalyzer()
:	Analyzer(),  
	mSettings( new IEBusAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

IEBusAnalyzer::~IEBusAnalyzer()
{
	KillThread();
}

void IEBusAnalyzer::WorkerThread()
{
	mResults.reset( new IEBusAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );

	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

	double tolerance = mSettings->mStartBitWidth * .05;

	// to hold the start of the start bit
	U64 start_sample_number_start;
	// to hold the finish of the start bit
	U64 start_sample_number_finish;

	if( mSerial->GetBitState() == BIT_LOW )
		mSerial->AdvanceToNextEdge();

	for( ; ; )
	{
		// flag for starting bit
		bool found_start = false;
		start_sample_number_start = mSerial->GetSampleNumber();

		// search for the starting bit
		while(!found_start){
			// go to edge of what might be the starting bit and get the sample number
			mSerial->AdvanceToNextEdge();
			start_sample_number_finish = mSerial->GetSampleNumber();

			// measure the sample and normilize the units
			U32 measure_width = (start_sample_number_finish - start_sample_number_start) * 2;
			if (measure_width > 100000)
				measure_width *= .01;
			else
				measure_width *= .001;

			// check to see if we have a starting bit
			// if so let us continue to collect data
			// if not we will reset the starting position
			if( ( measure_width ) > ( mSettings->mStartBitWidth - tolerance )
				&& ( measure_width ) < ( mSettings->mStartBitWidth + tolerance ) ) 
			{
				found_start = true;
				mResults->AddMarker( start_sample_number_start, AnalyzerResults::UpArrow, mSettings->mInputChannel );
				mResults->AddMarker( start_sample_number_finish, AnalyzerResults::Square, mSettings->mInputChannel );
			}
			else
			{
				mSerial->AdvanceToNextEdge();
				start_sample_number_start = mSerial->GetSampleNumber();
			}
		}

		bool finised_transmission = false;

		Frame frame;
		frame.mData1 = 0;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = start_sample_number_start;
		frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );

		// move to next rising edge
		mSerial->AdvanceToNextEdge();
	}
}

bool IEBusAnalyzer::NeedsRerun()
{
	return false;
}

U32 IEBusAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 IEBusAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mStartBitWidth * 4;
}

const char* IEBusAnalyzer::GetAnalyzerName() const
{
	return "IEbus by github.com/james-tate";
}

const char* GetAnalyzerName()
{
	return "IEbus by github.com/james-tate";
}

Analyzer* CreateAnalyzer()
{
	return new IEBusAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}