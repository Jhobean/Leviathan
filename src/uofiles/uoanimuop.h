#ifndef UOANIMUOP_H
#define UOANIMUOP_H

#include "uoppackage/UOPPackage.h"
#include <string>
#include <vector>
#include <functional>   // for std::function (callback)


class QImage;

class UOAnimUOP
{
public:
    UOAnimUOP(std::string clientPath, std::function<void(int)> reportProgress);
    //~UOAnimUOP();

    bool isLoading() const {
        return m_isLoading;
    }
    bool animExists(int animID);
    QImage* drawAnimFrame(int bodyID, int action, int direction, int frame, int hueIndex);

private:
    struct UOPAnimationData
    {
        int animFileIdx;        // AnimationFrame(x).uop
        size_t blockIdx;
        size_t fileIdx;
        unsigned long long hash;
    };

    struct UOPFrameData
    {
        size_t dataStart = 0;
        //uint_fast16_t frameId;
        unsigned int pixelDataOffset = 0;
    };

    std::string m_clientPath;
    uoppackage::UOPPackage m_animUOPs[4];

    std::vector<UOPAnimationData> m_animationsData;

    // sort animationsData by [animID][groupID]:
    static const int kAnimIdMax = 2048;   // animation ID
    static const int kGroupIdMax = 100;   // action ID
    UOPAnimationData* m_animationsMatrix[kAnimIdMax][kGroupIdMax];
    bool m_isLoading;

    void buildAnimTable(std::function<void(int)> reportProgress);
    UOPFrameData loadFrameData(int animID, int groupID, int direction, int frame, char *&decData, size_t& decDataSize); // decData: buffer for decompressed anim data
};

#endif // UOANIMUOP_H
