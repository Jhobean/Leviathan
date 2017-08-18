#include "uoanim.h"


UOAnim::UOAnim(std::string clientPath) :
    m_UOAnimMul(clientPath), m_UOAnimUOP(clientPath)
{
}

QImage* UOAnim::drawAnimFrame(int bodyID, int action, int direction, int frame, int hue_index)
{
    if (m_UOAnimUOP.animExists(bodyID))
        return m_UOAnimUOP.drawAnimFrame(bodyID, action, direction, frame, hue_index);
    else
        return m_UOAnimMul.drawAnimFrame(bodyID, action, direction, frame, hue_index);
}
