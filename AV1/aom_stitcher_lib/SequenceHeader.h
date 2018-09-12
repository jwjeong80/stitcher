#include "av1/decoder/obu.h"

class ShManager
{
public:
	ShManager();
	~ShManager();

	void		Init();
	void		Destroy();

	void		storeSequenceHeader(SequenceHeader *seqHeader);

	SequenceHeader*	getStoredVPS(int id) { return	m_ShBuffer[0]; }

	SequenceHeader*	getSequenceHeader() { return	m_ShBuffer[0]; };

private:

	SequenceHeader*	m_ShBuffer[1];

	int			m_ShId;
};
