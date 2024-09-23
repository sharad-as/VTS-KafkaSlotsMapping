/*
 * KafkaSlotsMapping.h
 *
 *  Created on: 13-Sept-2024
 *      Author: root
 */

#ifndef KAFKASLOTSMAPPING_H_
#define KAFKASLOTSMAPPING_H_

#include <iostream>
#include <vector>
#include "../../common/defines.h"
#include "../../common/struct.h"
#include "../../common/stl/unordered_map.h"
#include "../../common/reader_writer_lock.h"


using namespace std;

class CSlotsMapping
{
	vector<string>m_vecTopics;
	vector<tagSlotDetails*> m_vecSlots;
	int m_nTotalSlots;
	CATSUMap<string,tagSlotDetails*> m_mapSlotMappings;
	CRWLock m_cRWLock;
public:
	CSlotsMapping() = delete;
	CSlotsMapping(int f_nSlotCount)
	{
		m_nTotalSlots = 0;
	}
	short AddUpdateSlot(tagSlotDetails* f_pstSlotDetails);
	const tagSlotDetails* GetSlotDetails(char *f_pszKey);
};



#endif /* KAFKASLOTSMAPPING_H_ */
