//============================================================================
// Name        : KafkaSlotsMapping.cpp
// Author      : SS
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "../headers/KafkaSlotsMapping.h"
#include "../../common/enum.h"


//
//int main() {
//
//	return 0;
//}


// Function to calculate CRC-16 (CRC-16-CCITT XModem)
uint16_t crc16_ccitt(const std::vector<uint8_t>& data) {
    uint16_t crc = 0xFFFF; // initial value
    const uint16_t polynomial = 0x1021; // polynomial for CRC-16-CCITT

    for (const auto& byte : data) {
        crc ^= (byte << 8); // XOR byte into the upper 8 bits of crc

        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000) { // Check if the most significant bit is set
                crc = (crc << 1) ^ polynomial; // Shift left and XOR with the polynomial
            } else {
                crc <<= 1; // Just shift left if no overflow
            }
        }
    }

    return crc;
}
// Function to get the CRC-16 value of a string
uint16_t get_crc16_of_string(const std::string& input) {
    // Convert string to a vector of uint8_t (bytes)
    std::vector<uint8_t> data(input.begin(), input.end());

    // Calculate and return CRC-16
    return crc16_ccitt(data);
}

extern "C"{

HANDLE InitializeKafkaSlotsMapping(int f_nSlotCount)
{
	if(0 < f_nSlotCount)
	{
		CSlotsMapping *lpcSlotMapping = new CSlotsMapping(f_nSlotCount);
		return lpcSlotMapping;
	}
	return nullptr;
}

short AddUpdateSlot(HANDLE f_hSlotHandle, tagSlotDetails* f_pstSlotDetails)
{
	if(nullptr != f_hSlotHandle && f_pstSlotDetails)
	{
		((CSlotsMapping*)f_hSlotHandle)->AddUpdateSlot(f_pstSlotDetails);
		return (short)eReturnType::FAILURE;
	}
	else
	{
		return (short)eReturnType::FAILURE;
	}
}

const tagSlotDetails* GetSlotDetails(HANDLE f_hSlotHandle, char *f_pszEntityId)
{
	if(nullptr != f_hSlotHandle)
	{
		return ((CSlotsMapping*)f_hSlotHandle)->GetSlotDetails(f_pszEntityId);
	}
	return nullptr;
}
}

short CSlotsMapping::AddUpdateSlot(tagSlotDetails* f_pstSlotDetails)
{
	if(nullptr != f_pstSlotDetails)
	{
		m_cRWLock.AcquireWriteLock();
		auto lItr = find(m_vecTopics.begin(), m_vecTopics.end(),f_pstSlotDetails->szTopicName);
		if(lItr == m_vecTopics.end())
		{
			//Topic not found, create a new slot
			m_nTotalSlots++;
			tagSlotDetails *lpstSlotDetails = new tagSlotDetails();
			memcpy(lpstSlotDetails,f_pstSlotDetails, sizeof(tagSlotDetails));
			m_vecSlots.push_back(lpstSlotDetails);
			m_cRWLock.ReleaseWriteLock();
			cout<<"Adding new slot for topic "<< f_pstSlotDetails->szTopicName<< ", Total Slots: "<<m_nTotalSlots<<endl;
			m_vecTopics.push_back(f_pstSlotDetails->szTopicName);
			return (short)eReturnType::SUCCESS;
		}
		else
		{
			int lnIndex = distance(m_vecTopics.begin(),lItr);
			cout<<"record already available at "<<lnIndex<< " index."<<endl;
			m_cRWLock.ReleaseWriteLock();
			return (short)eReturnType::FAILURE;
		}
	}
	return (short)eReturnType::FAILURE;
}

const tagSlotDetails* CSlotsMapping::GetSlotDetails(char *f_pszKey)
{
	if(nullptr != f_pszKey)
	{
		tagSlotDetails *lpstSlotDetails = nullptr;
		m_cRWLock.AcquireWriteLock();
		m_mapSlotMappings.Find(f_pszKey,lpstSlotDetails);
		if(nullptr != lpstSlotDetails)
		{
			m_cRWLock.ReleaseWriteLock();
			return lpstSlotDetails;
		}
		else
		{
			//Get CRC 16 logical slot.
			uint16_t lnCRC = get_crc16_of_string(f_pszKey);
			short lnSlot = lnCRC%m_nTotalSlots;
			//std::cout << "CRC-16-CCITT of \"" << f_pszKey << "\": 0x" << std::hex << lnCRC <<",Total slots: "<<m_nTotalSlots<< std::endl;
			//cout<<"key: "<< f_pszKey<<", Slot: "<<lnSlot<<endl;
			if(lnSlot < m_nTotalSlots && nullptr != m_vecSlots[lnSlot])
			{
				lpstSlotDetails = m_vecSlots[lnSlot];
				m_mapSlotMappings.Insert(f_pszKey,lpstSlotDetails);
				m_cRWLock.ReleaseWriteLock();
				return lpstSlotDetails;
			}
			else
			{
				m_cRWLock.ReleaseWriteLock();
			}
		}
	}
	return nullptr;
}
