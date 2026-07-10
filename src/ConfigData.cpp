#include "ConfigData.h"

// Definition of the standard constructor
ConfigData::ConfigData() {
  isValid = false; // Default object is not valid
}

// Definition of the intelligent constructor
ConfigData::ConfigData(String rawMessage) {
  rawMessage.trim(); // Remove invisible characters like \n or trailing spaces

  // Find semicolons in the CSV string
  int index0 = rawMessage.indexOf(';');
  int index1 = rawMessage.indexOf(';', index0 + 1);
  int index2 = rawMessage.indexOf(';', index1 + 1);
  int index3 = rawMessage.indexOf(';', index2 + 1);
  int index4 = rawMessage.indexOf(';', index3 + 1);
  int index5 = rawMessage.indexOf(';', index4 + 1);
  int index6 = rawMessage.indexOf(';', index5 + 1);
  int index7 = rawMessage.indexOf(';', index6 + 1);
  int index8 = rawMessage.indexOf(';', index7 + 1);
  int index9 = rawMessage.indexOf(';', index8 + 1);

  // Validation: If any semicolon is missing, the string is corrupted
  if (index0 == -1 || index1 == -1 || index2 == -1 || index3 == -1 || index4 == -1 || index5 == -1 || index6 == -1 || index7 == -1 || index8 == -1 || index9 == -1) {
    isValid = false;
    return; 
  }

  // Populate own variables with extracted values
  digit        = rawMessage.substring(0, index0).toInt();
  mode         = rawMessage.substring(index0 + 1, index1);
  durationMs   = rawMessage.substring(index1 + 1, index2).toInt();
  intensityPct = rawMessage.substring(index2 + 1, index3).toInt();
  postDigitDelayMs = rawMessage.substring(index3 + 1, index4).toInt(); 
  dscrStartHoldFactor = rawMessage.substring(index4 + 1, index5).toFloat();
  dscrEndHoldFactor = rawMessage.substring(index5 + 1, index6).toFloat();
  atmStartHoldFactor = rawMessage.substring(index6 + 1, index7).toFloat();
  atmEndHoldFactor = rawMessage.substring(index7 + 1, index8).toFloat();
  ptsStartHoldFactor = rawMessage.substring(index8 + 1, index9).toFloat();
  ptsEndHoldFactor = rawMessage.substring(index9 + 1).toFloat();
  // Set to true if parsing succeeded completely
  isValid = true;
}