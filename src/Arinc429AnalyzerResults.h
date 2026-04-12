#ifndef ARINC429_ANALYZER_RESULTS_H
#define ARINC429_ANALYZER_RESULTS_H

#include <AnalyzerResults.h>

// mType values stored in Frame::mType
#define ARINC429_FRAME_TYPE_VALID        0
#define ARINC429_FRAME_TYPE_PARITY_ERROR 1

class Arinc429Analyzer;
class Arinc429AnalyzerSettings;

class Arinc429AnalyzerResults : public AnalyzerResults
{
public:
    Arinc429AnalyzerResults( Arinc429Analyzer* analyzer, Arinc429AnalyzerSettings* settings );
    virtual ~Arinc429AnalyzerResults();

    virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
    virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

    virtual void GenerateFrameTabularText( U64 frame_index, DisplayBase display_base );
    virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
    virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

protected:
    Arinc429AnalyzerSettings* mSettings;
    Arinc429Analyzer* mAnalyzer;
};

#endif // ARINC429_ANALYZER_RESULTS_H
