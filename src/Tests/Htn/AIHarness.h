//
//  AIHarness.h
//  TestLib
//
//  Created by Eric Zinda on 4/10/19.
//  Copyright © 2019 Eric Zinda. All rights reserved.
//

#ifndef AIHarness_h
#define AIHarness_h

#include "FailFast.h"
#include "FXPlatform/Directory.h"
#include "FXPlatform/FileStream.h"
class AIHarness
{
    public:
    AIHarness(const std::string &testName)
    {
        FailFastAssert(testName.size() > 0);
        m_testName = testName;
    }
    
    void AddResult(const std::string &resultName, const std::string &result)
    {
        m_currentResult = m_currentResult + "[" + resultName + "] => " + result + "\r\n\r\n";
    }
    
    bool CompareResult()
    {
        std::string baselinePath = Directory::InstallationFolder()->FullPathFromLocalPath(std::string("BaselineAI/") + m_testName + "-Baseline.txt");
        std::string baselineData;
        if(Directory::FileExists(baselinePath))
        {
            FileStream baselineStream;
            baselineStream.Open(baselinePath, FileOpenStyle::OpenExisting, AccessRights::Read);
            baselineData = baselineStream.ReadAll();
        }
        
        bool fileMatched = m_currentResult.compare(baselineData) == 0;

        if(!fileMatched)
        {
            // Write out the results
            FileStream resultsStream;
            std::string resultsPath = Directory::ApplicationFolder()->FullPathFromLocalPath(m_testName + "-Results.txt");
            resultsStream.Open(resultsPath, FileOpenStyle::CreateAlways, AccessRights::Write);
            resultsStream.Write(m_currentResult);
            
            // Copy the baseline too if it exists
            if(baselineData.size() > 0)
            {
                Directory::CopyFile(baselinePath, Directory::ApplicationFolder()->FullPathFromLocalPath(m_testName + "-Baseline.txt"));
            }
        }
                                    
        return fileMatched;
    }
    
    std::string m_testName;
    std::string m_currentResult;
};
#endif /* AIHarness_h */
