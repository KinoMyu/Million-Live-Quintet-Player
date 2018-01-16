
//--------------------------------------------------
// インクルード
//--------------------------------------------------
#include "clHCA.h"
#include <stdio.h>
#include <memory.h>
#include <utility>

//--------------------------------------------------
// インライン関数
//--------------------------------------------------
inline short bswap(short v) { short r = v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; return r; }
inline unsigned short bswap(unsigned short v) { unsigned short r = v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; return r; }
inline int bswap(int v) { int r = v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; return r; }
inline unsigned int bswap(unsigned int v) { unsigned int r = v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; return r; }
inline long long bswap(long long v) { long long r = v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; return r; }
inline unsigned long long bswap(unsigned long long v) { unsigned long long r = v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; r <<= 8; v >>= 8; r |= v & 0xFF; return r; }
inline float bswap(float v) { unsigned int i = bswap(*(unsigned int *)&v); return *(float *)&i; }
inline unsigned int ceil2(unsigned int a, unsigned int b) { return (b>0) ? (a / b + ((a%b) ? 1 : 0)) : 0; }

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
clHCA::clHCA(unsigned int ciphKey1, unsigned int ciphKey2) :
    _ciph_key1(ciphKey1), _ciph_key2(ciphKey2), _ath(), _cipher() {
    hcafileptr = nullptr;
}

clHCA & clHCA::operator=(clHCA && other)
{
    if(hcafileptr != nullptr)
    {
        delete[] hcafileptr;
    }
    hcafileptr = other.hcafileptr;
    other.hcafileptr = nullptr;
    _version = other._version;
    _dataOffset = other._dataOffset;
    _channelCount = other._channelCount;
    _samplingRate = other._samplingRate;
    _blockCount = other._blockCount;
    _muteHeader = other._muteHeader;
    _muteFooter = other._muteFooter;
    _blockSize = other._blockSize;
    _comp_r01 = other._comp_r01;
    _comp_r02 = other._comp_r02;
    _comp_r03 = other._comp_r03;
    _comp_r04 = other._comp_r04;
    _comp_r05 = other._comp_r05;
    _comp_r06 = other._comp_r06;
    _comp_r07 = other._comp_r07;
    _comp_r08 = other._comp_r08;
    _comp_r09 = other._comp_r09;
    _vbr_r01 = other._vbr_r01;
    _vbr_r02 = other._vbr_r02;
    _ath_type = other._ath_type;
    _loopStart = other._loopStart;
    _loopEnd = other._loopEnd;
    _loopCount = other._loopCount;
    _loop_r01 = other._loop_r01;
    _loopFlg = other._loopFlg;
    _ciph_type = other._ciph_type;
    _ciph_key1 = other._ciph_key1;
    _ciph_key2 = other._ciph_key2;
    _rva_volume = other._rva_volume;
    _comm_len = other._comm_len;
    _ath = other._ath;
    _cipher = other._cipher;
    return *this;
}

clHCA::~clHCA()
{
    if (hcafileptr != nullptr)
    {
        delete[] hcafileptr;
    }
}

//--------------------------------------------------
// HCAチェック
//--------------------------------------------------
bool clHCA::CheckFile(void *data, unsigned int size) {
    return (data&&size >= 4 && (*(unsigned int *)data & 0x7F7F7F7F) == 0x00414348);
}

//--------------------------------------------------
// チェックサム
//--------------------------------------------------
unsigned short clHCA::CheckSum(void *data, int size, unsigned short sum) {
    static unsigned short v[] = {
        0x0000,0x8005,0x800F,0x000A,0x801B,0x001E,0x0014,0x8011,0x8033,0x0036,0x003C,0x8039,0x0028,0x802D,0x8027,0x0022,
        0x8063,0x0066,0x006C,0x8069,0x0078,0x807D,0x8077,0x0072,0x0050,0x8055,0x805F,0x005A,0x804B,0x004E,0x0044,0x8041,
        0x80C3,0x00C6,0x00CC,0x80C9,0x00D8,0x80DD,0x80D7,0x00D2,0x00F0,0x80F5,0x80FF,0x00FA,0x80EB,0x00EE,0x00E4,0x80E1,
        0x00A0,0x80A5,0x80AF,0x00AA,0x80BB,0x00BE,0x00B4,0x80B1,0x8093,0x0096,0x009C,0x8099,0x0088,0x808D,0x8087,0x0082,
        0x8183,0x0186,0x018C,0x8189,0x0198,0x819D,0x8197,0x0192,0x01B0,0x81B5,0x81BF,0x01BA,0x81AB,0x01AE,0x01A4,0x81A1,
        0x01E0,0x81E5,0x81EF,0x01EA,0x81FB,0x01FE,0x01F4,0x81F1,0x81D3,0x01D6,0x01DC,0x81D9,0x01C8,0x81CD,0x81C7,0x01C2,
        0x0140,0x8145,0x814F,0x014A,0x815B,0x015E,0x0154,0x8151,0x8173,0x0176,0x017C,0x8179,0x0168,0x816D,0x8167,0x0162,
        0x8123,0x0126,0x012C,0x8129,0x0138,0x813D,0x8137,0x0132,0x0110,0x8115,0x811F,0x011A,0x810B,0x010E,0x0104,0x8101,
        0x8303,0x0306,0x030C,0x8309,0x0318,0x831D,0x8317,0x0312,0x0330,0x8335,0x833F,0x033A,0x832B,0x032E,0x0324,0x8321,
        0x0360,0x8365,0x836F,0x036A,0x837B,0x037E,0x0374,0x8371,0x8353,0x0356,0x035C,0x8359,0x0348,0x834D,0x8347,0x0342,
        0x03C0,0x83C5,0x83CF,0x03CA,0x83DB,0x03DE,0x03D4,0x83D1,0x83F3,0x03F6,0x03FC,0x83F9,0x03E8,0x83ED,0x83E7,0x03E2,
        0x83A3,0x03A6,0x03AC,0x83A9,0x03B8,0x83BD,0x83B7,0x03B2,0x0390,0x8395,0x839F,0x039A,0x838B,0x038E,0x0384,0x8381,
        0x0280,0x8285,0x828F,0x028A,0x829B,0x029E,0x0294,0x8291,0x82B3,0x02B6,0x02BC,0x82B9,0x02A8,0x82AD,0x82A7,0x02A2,
        0x82E3,0x02E6,0x02EC,0x82E9,0x02F8,0x82FD,0x82F7,0x02F2,0x02D0,0x82D5,0x82DF,0x02DA,0x82CB,0x02CE,0x02C4,0x82C1,
        0x8243,0x0246,0x024C,0x8249,0x0258,0x825D,0x8257,0x0252,0x0270,0x8275,0x827F,0x027A,0x826B,0x026E,0x0264,0x8261,
        0x0220,0x8225,0x822F,0x022A,0x823B,0x023E,0x0234,0x8231,0x8213,0x0216,0x021C,0x8219,0x0208,0x820D,0x8207,0x0202,
    };
    for (unsigned char *s = (unsigned char *)data, *e = s + size; s<e; s++)sum = (sum << 8) ^ v[(sum >> 8) ^ *s];
    return sum;
}

//--------------------------------------------------
// ヘッダ情報をコンソール出力
//--------------------------------------------------
bool clHCA::PrintInfo(const char *filenameHCA) {

    // チェック
    if (!(filenameHCA))return false;

    // HCAファイルを開く
    FILE *fp;
    if (fopen_s(&fp, filenameHCA, "rb")) {
        printf("Error: ファイルが開けませんでした。\n");
        return false;
    }

    // ヘッダチェック
    stHeader header;
    memset(&header, 0, sizeof(header));
    fread(&header, sizeof(header), 1, fp);
    if (!CheckFile(&header, sizeof(header))) {
        printf("Error: HCAファイルではありません。\n");
        fclose(fp); return false;
    }

    // ヘッダ解析
    header.dataOffset = bswap(header.dataOffset);
    unsigned char *data = new unsigned char[header.dataOffset];
    if (!data) {
        printf("Error: メモリ不足です。\n");
        fclose(fp); return false;
    }
    fseek(fp, 0, SEEK_SET);
    fread(data, header.dataOffset, 1, fp);

    unsigned char *s = (unsigned char *)data;
    unsigned int size = header.dataOffset;

    // サイズチェック
    if (size<sizeof(stHeader)) {
        printf("Error: ヘッダのサイズが小さすぎます。\n");
        delete[] data; fclose(fp); return false;
    }

    // HCA
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00414348) {
        stHeader *hca = (stHeader *)s; s += sizeof(stHeader);
        _version = bswap(hca->version);
        _dataOffset = bswap(hca->dataOffset);
        printf("コーデック: HCA\n");
        printf("バージョン: %d.%d\n", _version >> 8, _version & 0xFF);
        //if(size<_dataOffset)return false;
        if (CheckSum(hca, _dataOffset))printf("※ ヘッダが破損しています。改変してる場合もこの警告が出ます。\n");
    }
    else {
        printf("※ HCAチャンクがありません。再生に必要な情報です。\n");
    }

    // fmt
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00746D66) {
        stFormat *fmt = (stFormat *)s; s += sizeof(stFormat);
        _channelCount = fmt->channelCount;
        _samplingRate = bswap(fmt->samplingRate << 8);
        _blockCount = bswap(fmt->blockCount);
        _muteHeader = bswap(fmt->muteHeader);
        _muteFooter = bswap(fmt->muteFooter);
        switch (_channelCount) {
        case 1:printf("チャンネル数: モノラル (1チャンネル)\n"); break;
        case 2:printf("チャンネル数: ステレオ (2チャンネル)\n"); break;
        default:printf("チャンネル数: %dチャンネル\n", _channelCount); break;
        }
        if (!(_channelCount >= 1 && _channelCount <= 16)) {
            printf("※ チャンネル数の範囲は1～16です。\n");
        }
        printf("サンプリングレート: %dHz\n", _samplingRate);
        if (!(_samplingRate >= 1 && _samplingRate <= 0x7FFFFF)) {
            printf("※ サンプリングレートの範囲は1～8388607(0x7FFFFF)です。\n");
        }
        printf("ブロック数: %d\n", _blockCount);
        printf("先頭無音ブロック数: %d\n", (_muteHeader - 0x80) / 0x400);
        printf("末尾無音サンプル数: %d\n", _muteFooter);
    }
    else {
        printf("※ fmtチャンクがありません。再生に必要な情報です。\n");
    }

    // comp
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706D6F63) {
        stCompress *comp = (stCompress *)s; s += sizeof(stCompress);
        _blockSize = bswap(comp->blockSize);
        _comp_r01 = comp->r01;
        _comp_r02 = comp->r02;
        _comp_r03 = comp->r03;
        _comp_r04 = comp->r04;
        _comp_r05 = comp->r05;
        _comp_r06 = comp->r06;
        _comp_r07 = comp->r07;
        _comp_r08 = comp->r08;
        unsigned int bps = _samplingRate * _blockSize / 128;
        if (bps<1000000)printf("ビットレート: %gkbps CBR (固定ビットレート)\n", bps / 1000.0f);
        else printf("ビットレート: %gMbps CBR (固定ビットレート)\n", bps / 1000000.0f);
        printf("ブロックサイズ: 0x%X\n", _blockSize);
        if (!(_blockSize >= 8 && _blockSize <= 0xFFFF)) {
            printf("※ ブロックサイズの範囲は8～65535(0xFFFF)です。v1.3では0でVBRになるようになってましたが、v2.0から廃止されたようです。\n");
        }
        printf("comp1: %d\n", _comp_r01);
        printf("comp2: %d\n", _comp_r02);
        if (!(_comp_r01 >= 0 && _comp_r01 <= _comp_r02 && _comp_r02 <= 0x1F)) {
            printf("※ comp1とcomp2の範囲は0<=comp1<=comp2<=31です。v2.0現在、comp1は1、comp2は15で固定されています。\n");
        }
        printf("comp3: %d\n", _comp_r03);
        if (!_comp_r03) {
            printf("※ comp3は1以上の値です。\n");
        }
        printf("comp4: %d\n", _comp_r04);
        printf("comp5: %d\n", _comp_r05);
        printf("comp6: %d\n", _comp_r06);
        printf("comp7: %d\n", _comp_r07);
        printf("comp8: %d\n", _comp_r08);
    }

    // dec
    else if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00636564) {
        stDecode *dec = (stDecode *)s; s += sizeof(stDecode);
        _blockSize = bswap(dec->blockSize);
        _comp_r01 = dec->r01;
        _comp_r02 = dec->r02;
        _comp_r03 = dec->r04;
        _comp_r04 = dec->r03;
        _comp_r05 = dec->count1 + 1;
        _comp_r06 = ((dec->enableCount2) ? dec->count2 : dec->count1) + 1;
        _comp_r07 = _comp_r05 - _comp_r06;
        _comp_r08 = 0;
        unsigned int bps = _samplingRate * _blockSize / 128;
        if (bps<1000000)printf("ビットレート: %gkbps CBR (固定ビットレート)\n", bps / 1000.0f);
        else printf("ビットレート: %gMbps CBR (固定ビットレート)\n", bps / 1000000.0f);
        printf("ブロックサイズ: 0x%X\n", _blockSize);
        if (!(_blockSize >= 8 && _blockSize <= 0xFFFF)) {
            printf("※ ブロックサイズの範囲は8～65535(0xFFFF)です。v1.3では0でVBRになるようになってましたが、v2.0から廃止されたようです。\n");
        }
        printf("dec1: %d\n", _comp_r01);
        printf("dec2: %d\n", _comp_r02);
        if (!(_comp_r01 >= 0 && _comp_r01 <= _comp_r02 && _comp_r02 <= 0x1F)) {
            printf("※ dec1とdec2の範囲は0<=dec1<=dec2<=31です。v2.0現在、dec1は1、dec2は15で固定されています。\n");
        }
        printf("dec3: %d\n", _comp_r03);
        if (!_comp_r03) {
            printf("※ dec3は再生時に1以上の値に修正されます。\n");
        }
        printf("dec4: %d\n", _comp_r04);
        printf("dec5: %d\n", _comp_r05);
        printf("dec6: %d\n", _comp_r06);
        printf("dec7: %d\n", _comp_r07);
    }
    else {
        printf("※ compチャンクまたはdecチャンクがありません。再生に必要な情報です。\n");
    }

    // vbr
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00726276) {
        stVBR *vbr = (stVBR *)s; s += sizeof(stVBR);
        _vbr_r01 = bswap(vbr->r01);
        _vbr_r02 = bswap(vbr->r02);
        printf("ビットレート: VBR (可変ビットレート) ※v2.0で廃止されています。\n");
        if (!(_blockSize == 0)) {
            printf("※ compまたはdecチャンクですでにCBRが指定されています。\n");
        }
        printf("vbr1: %d\n", _vbr_r01);
        if (!(_vbr_r01 >= 0 && _vbr_r01 <= 0x1FF)) {
            printf("※ vbr1の範囲は0～511(0x1FF)です。\n");
        }
        printf("vbr2: %d\n", _vbr_r02);
    }
    else {
        _vbr_r01 = 0;
        _vbr_r02 = 0;
    }

    // ath
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00687461) {
        stATH *ath = (stATH *)s; s += 6;//s+=sizeof(stATH);
        _ath_type = ath->type;
        printf("ATHタイプ:%d ※v2.0から廃止されています。\n", _ath_type);
    }
    else {
        if (_version<0x200) {
            printf("ATHタイプ:1 ※v2.0から廃止されています。\n");
        }
    }

    // loop
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706F6F6C) {
        stLoop *loop = (stLoop *)s; s += sizeof(stLoop);
        _loopStart = bswap(loop->start);
        _loopEnd = bswap(loop->end);
        _loopCount = bswap(loop->count);
        _loop_r01 = bswap(loop->r01);
        printf("ループ開始ブロック: %d\n", _loopStart);
        printf("ループ終了ブロック: %d\n", _loopEnd);
        if (!(_loopStart >= 0 && _loopStart <= _loopEnd && _loopEnd<_blockCount)) {
            printf("※ ループ開始ブロックとループ終了ブロックの範囲は、0<=ループ開始ブロック<=ループ終了ブロック<ブロック数 です。\n");
        }
        if (_loopCount == 0x80) {
            printf("ループ回数: 無限ループ\n");
        }
        else {
            printf("ループ回数: %d回\n", _loopCount);
        }
        printf("ループ情報1: %d\n", _loop_r01);
    }

    // ciph
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x68706963) {
        stCipher *ciph = (stCipher *)s; s += 6;//s+=sizeof(stCipher);
        _ciph_type = bswap(ciph->type);
        switch (_ciph_type) {
        case 0:printf("暗号化タイプ: なし\n"); break;
        case 1:printf("暗号化タイプ: 鍵無し暗号化\n"); break;
        case 0x38:printf("暗号化タイプ: 鍵有り暗号化 ※正しい鍵を使わないと出力波形がおかしくなります。\n"); break;
        default:printf("暗号化タイプ: %d\n", _ciph_type); break;
        }
        if (!(_ciph_type == 0 || _ciph_type == 1 || _ciph_type == 0x38)) {
            printf("※ この暗号化タイプは、v2.0現在再生できません。\n");
        }
    }

    // rva
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00617672) {
        stRVA *rva = (stRVA *)s; s += sizeof(stRVA);
        _rva_volume = bswap(rva->volume);
        printf("相対ボリューム調節: %g倍\n", _rva_volume);
    }

    // comm
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x6D6D6F63) {
        stComment *comm = (stComment *)s; s += 5;//s+=sizeof(stComment);
        _comm_len = comm->len;
        _comm_comment = (char *)s;
        printf("コメント: %s\n", _comm_comment);
    }

    delete[] data;

    // 閉じる
    fclose(fp);

    return true;
}

//--------------------------------------------------
// 復号化
//--------------------------------------------------
bool clHCA::Decrypt(const char *filenameHCA) {

    // チェック
    if (!(filenameHCA))return false;

    // HCAファイルを開く
    FILE *fp;
    if (fopen_s(&fp, filenameHCA, "r+b"))return false;

    // ヘッダチェック
    stHeader header;
    memset(&header, 0, sizeof(header));
    fread(&header, sizeof(header), 1, fp);
    if (!CheckFile(&header, sizeof(header))) { fclose(fp); return false; }

    // ヘッダ解析
    header.dataOffset = bswap(header.dataOffset);
    unsigned char *data = new unsigned char[header.dataOffset];
    if (!data) { fclose(fp); return false; }
    fseek(fp, 0, SEEK_SET);
    fread(data, header.dataOffset, 1, fp);

    unsigned char *s = (unsigned char *)data;
    unsigned int size = header.dataOffset;

    // サイズチェック
    if (size<sizeof(stHeader)) { delete[] data; fclose(fp); return false; }

    // HCA
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00414348) {
        stHeader *hca = (stHeader *)s; s += sizeof(stHeader);
        hca->hca &= 0x7F7F7F7F;
        _version = bswap(hca->version);
        _dataOffset = bswap(hca->dataOffset);
    }
    else {
        delete[] data; fclose(fp); return false;
    }

    // fmt
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00746D66) {
        stFormat *fmt = (stFormat *)s; s += sizeof(stFormat);
        fmt->fmt &= 0x7F7F7F7F;
        _samplingRate = bswap(fmt->samplingRate << 8);
        _blockCount = bswap(fmt->blockCount);
    }
    else {
        delete[] data; fclose(fp); return false;
    }

    // comp
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706D6F63) {
        stCompress *comp = (stCompress *)s; s += sizeof(stCompress);
        comp->comp &= 0x7F7F7F7F;
        _blockSize = bswap(comp->blockSize);
    }

    // dec
    else if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00636564) {
        stDecode *dec = (stDecode *)s; s += sizeof(stDecode);
        dec->dec &= 0x7F7F7F7F;
        _blockSize = bswap(dec->blockSize);
    }
    else {
        delete[] data; fclose(fp); return false;
    }

    // vbr
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00726276) {
        stVBR *vbr = (stVBR *)s; s += sizeof(stVBR);
        vbr->vbr &= 0x7F7F7F7F;
    }

    // ath
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00687461) {
        stATH *ath = (stATH *)s; s += 6;//s+=sizeof(stATH);
        ath->ath &= 0x7F7F7F7F;
        _ath_type = bswap(ath->type);
        //ath->type=0;
    }
    else {
        if (_version<0x200)_ath_type = 1;
    }

    // loop
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706F6F6C) {
        stLoop *loop = (stLoop *)s; s += sizeof(stLoop);
        loop->loop &= 0x7F7F7F7F;
    }

    // ciph
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x68706963) {
        stCipher *ciph = (stCipher *)s; s += 6;//s+=sizeof(stCipher);
        ciph->ciph &= 0x7F7F7F7F;
        _ciph_type = bswap(ciph->type);
        ciph->type = 0;
    }

    // rva
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00617672) {
        stRVA *rva = (stRVA *)s; s += sizeof(stRVA);
        rva->rva &= 0x7F7F7F7F;
    }

    // comm
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x6D6D6F63) {
        stComment *comm = (stComment *)s; s += 5;//s+=sizeof(stComment);
        comm->comm &= 0x7F7F7F7F;
        s += comm->len;
    }

    // pad
    if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00646170) {
        stPadding *pad = (stPadding *)s; s += sizeof(stPadding);
        pad->pad &= 0x7F7F7F7F;
    }

    // 初期化
    if (!_ath.Init(_ath_type, _samplingRate)) { delete[] data; fclose(fp); return false; }
    if (!_cipher.Init(_ciph_type, _ciph_key1, _ciph_key2)) { delete[] data; fclose(fp); return false; }
    unsigned char *data2 = new unsigned char[_blockSize];
    if (!data2) { delete[] data; fclose(fp); return false; }

    // ヘッダを書き込み
    *(unsigned short *)&data[size - 2] = bswap(CheckSum(data, size - 2));
    fseek(fp, 0, SEEK_SET);
    fwrite(data, size, 1, fp);
    delete[] data;

    // ブロックデータを復号化
    if (_ciph_type != 0) {
        for (unsigned int i = 0, a = size; i<_blockCount; i++, a += _blockSize) {
            fseek(fp, a, SEEK_SET);
            fread(data2, _blockSize, 1, fp);
            _cipher.Mask(data2, _blockSize);
            *(unsigned short *)&data2[_blockSize - 2] = bswap(CheckSum(data2, _blockSize - 2));
            fseek(fp, a, SEEK_SET);
            fwrite(data2, _blockSize, 1, fp);
        }
    }
    delete[] data2;

    // 閉じる
    fclose(fp);

    return true;
}
//--------------------------------------------------
// Analyze and store information about HCA
//-------------------------------------------------
bool clHCA::Analyze(void*& wavptr, size_t& sz, const char* filenameHCA)
{
    wavptr = nullptr;
    sz = 0;
    // チェック
    if (!(filenameHCA))return false;

    // HCAファイルを開く
    FILE *fp;
    if (fopen_s(&fp, filenameHCA, "rb"))return false;

    // Analyze

    // チェック
    int mode = 16;
    int loop = 0;
    if (!(fp && (mode == 0 || mode == 8 || mode == 16 || mode == 24 || mode == 32) && loop >= 0))
    {
        fclose(fp);
        return false;
    }

    // 
    FILE *fp1 = (FILE *)fp;
    unsigned int address = ftell(fp1);

    // ヘッダチェック
    stHeader header;
    memset(&header, 0, sizeof(header));
    fread(&header, sizeof(header), 1, fp1);
    if (!CheckFile(&header, sizeof(header)))
    {
        fclose(fp);
        return false;
    }

    // ヘッダ解析
    header.dataOffset = bswap(header.dataOffset);
    unsigned char *data1 = new unsigned char[header.dataOffset];
    if (!data1) { fclose(fp1); return false; }
    fseek(fp1, address, SEEK_SET);
    fread(data1, header.dataOffset, 1, fp1);
    if (!Decode(data1, header.dataOffset, 0)) { delete[] data1;
    fclose(fp); return false; }

    // WAVEヘッダを書き込み
    struct stWAVEHeader {
        char riff[4];
        unsigned int riffSize;
        char wave[4];
        char fmt[4];
        unsigned int fmtSize;
        unsigned short fmtType;
        unsigned short fmtChannelCount;
        unsigned int fmtSamplingRate;
        unsigned int fmtSamplesPerSec;
        unsigned short fmtSamplingSize;
        unsigned short fmtBitCount;
    }wavRiff = { 'R','I','F','F',0,'W','A','V','E','f','m','t',' ',0x10,0,0,0,0,0,0 };
    struct stWAVEsmpl {
        char smpl[4];
        unsigned int smplSize;
        unsigned int manufacturer;
        unsigned int product;
        unsigned int samplePeriod;
        unsigned int MIDIUnityNote;
        unsigned int MIDIPitchFraction;
        unsigned int SMPTEFormat;
        unsigned int SMPTEOffset;
        unsigned int sampleLoops;
        unsigned int samplerData;
        unsigned int loop_Identifier;
        unsigned int loop_Type;
        unsigned int loop_Start;
        unsigned int loop_End;
        unsigned int loop_Fraction;
        unsigned int loop_PlayCount;
    }wavSmpl = { 's','m','p','l',0x3C,0,0,0,0x3C,0,0,0,1,0x18,0,0,0,0,0,0 };
    struct stWAVEnote {
        char note[4];
        unsigned int noteSize;
        unsigned int dwName;
    }wavNote = { 'n','o','t','e',0,0 };
    struct stWAVEdata {
        char data[4];
        unsigned int dataSize;
    }wavData = { 'd','a','t','a',0 };
    wavRiff.fmtType = (mode>0) ? 1 : 3;
    wavRiff.fmtChannelCount = _channelCount;
    wavRiff.fmtBitCount = (mode>0) ? mode : 32;
    wavRiff.fmtSamplingRate = _samplingRate;
    wavRiff.fmtSamplingSize = wavRiff.fmtBitCount / 8 * wavRiff.fmtChannelCount;
    wavRiff.fmtSamplesPerSec = wavRiff.fmtSamplingRate*wavRiff.fmtSamplingSize;
    if (_loopFlg) {
        wavSmpl.samplePeriod = (unsigned int)(1 / (double)wavRiff.fmtSamplingRate * 1000000000);
        wavSmpl.loop_Start = _loopStart * 0x80 * 8 + _muteFooter;//※計算方法不明
        wavSmpl.loop_End = (_loopEnd + 1) * 0x80 * 8 - 1;//※計算方法不明
        wavSmpl.loop_PlayCount = (_loopCount == 0x80) ? 0 : _loopCount;
    }
    else if (loop) {
        wavSmpl.loop_Start = 0;
        wavSmpl.loop_End = (_blockCount + 1) * 0x80 * 8 - 1;//※計算方法不明
        _loopStart = 0;
        _loopEnd = _blockCount;
    }
    if (_comm_comment) {
        wavNote.noteSize = 4 + _comm_len + 1;
        if (wavNote.noteSize & 3)wavNote.noteSize += 4 - (wavNote.noteSize & 3);
    }
    wavData.dataSize = _blockCount * 0x80 * 8 * wavRiff.fmtSamplingSize + (wavSmpl.loop_End - wavSmpl.loop_Start)*loop;
    wavRiff.riffSize = 0x1C + ((_loopFlg && !loop) ? sizeof(wavSmpl) : 0) + (_comm_comment ? 8 + wavNote.noteSize : 0) + sizeof(wavData) + wavData.dataSize;

    sz = _blockCount * 8 * 128 * _channelCount * 2 + sizeof(wavRiff) + sizeof(wavData);
    wavptr = new char[sz];
    memset(wavptr, 0, sz);
    int seekhead = 0;
    for (int i = 0; i < sizeof(wavRiff); ++i)
    {
        ((char*)wavptr)[seekhead++] = ((char*)&wavRiff)[i];
    }
    for (int i = 0; i < sizeof(wavData); ++i)
    {
        ((char*)wavptr)[seekhead++] = ((char*)(&wavData))[i];
    }
    delete[] data1;
    hcafileptr = new unsigned char[_blockCount * _blockSize];
    fread(hcafileptr, _blockCount, _blockSize, fp1);
    // 閉じる
    fclose(fp);
    return true;
}

void clHCA::AsyncDecode(stChannel* channelsOffset, unsigned int blocknum, void*& outputwavptr, unsigned int chunksize, Semaphore& wavoutsem)
{
    int seekhead = 0;
    char* outwavptr = (char*)outputwavptr + 2 * blocknum * 8 * 128 * _channelCount + 44;
    unsigned char* data = new unsigned char[_blockSize];
    int x = (blocknum == 0) ? 0 : -1;
    if(blocknum == 0)
    {
        PrepDecode(channelsOffset, 1);
    }
    for (; x < (int)chunksize && blocknum + x < _blockCount; ++x)
    {
        memcpy(data, (unsigned char*)hcafileptr + ((blocknum + x) * _blockSize), _blockSize);
        //        if(((unsigned char *)data)[_blockSize-2]==0x5E)_asm int 3
        _cipher.Mask(data, _blockSize);
        clData d(data, _blockSize);
        int magic = d.GetBit(16);//0xFFFF???
        if (magic == 0xFFFF) {
            int a = (d.GetBit(9) << 8) - d.GetBit(7);
            for (unsigned int i = 0; i < _channelCount; i++)channelsOffset[i].Decode1(&d, _comp_r09, a, _ath.GetTable());
            for (int i = 0; i < 8; i++) {
                for (unsigned int j = 0; j < _channelCount; j++)channelsOffset[j].Decode2(&d);
                for (unsigned int j = 0; j < _channelCount; j++)channelsOffset[j].Decode3(_comp_r09, _comp_r08, _comp_r07 + _comp_r06, _comp_r05);
                for (unsigned int j = 0; j < _channelCount - 1; j++)channelsOffset[j].Decode4(i, _comp_r05 - _comp_r06, _comp_r06, _comp_r07);
                for (unsigned int j = 0; j < _channelCount; j++)channelsOffset[j].Decode5(i);
                if (x >= 0)
                {
                    wavoutsem.wait();
                    if(outputwavptr == nullptr)
                    {
                        wavoutsem.notify();
                        delete[] data;
                        return;
                    }
                    for (int j = 0; j < 0x80; j++) {
                        for (unsigned int k = 0; k < _channelCount; k++) {
                            float f = channelsOffset[k].wave[i][j];
                            if (f > 1) { f = 1; }
                            else if (f < -1) { f = -1; }
                            DecodeToMemory_DecodeMode16bit(f, outwavptr, seekhead);
                        }
                    }
                    wavoutsem.notify();
                }
            }
        }
    }
    delete[] data;
}

//--------------------------------------------------
// デコードしてWAVEファイルに保存
//--------------------------------------------------
bool clHCA::DecodeToWavefile(const char *filenameHCA, const char *filenameWAV, float volume, int mode, int loop) {

    // チェック
    if (!(filenameHCA))return false;

    // HCAファイルを開く
    FILE *fp;
    if (fopen_s(&fp, filenameHCA, "rb"))return false;

    // 保存
    if (!DecodeToWavefileStream(fp, filenameWAV, volume, mode, loop)) { fclose(fp); return false; }

    // 閉じる
    fclose(fp);

    return true;
}
void* clHCA::DecodeToMemory(size_t& sz, const char *filenameHCA, float volume, int mode, int loop) {

    void* ptr = nullptr;
    // チェック
    if (!(filenameHCA))return nullptr;

    // HCAファイルを開く
    FILE *fp;
    if (fopen_s(&fp, filenameHCA, "rb"))return nullptr;

    // 保存
    ptr = DecodeToMemoryStream(sz, fp, volume, mode, loop);

    // 閉じる
    fclose(fp);

    return ptr;
}

void* clHCA::DecodeToMemoryStream(size_t& sz, void *fpHCA, float volume, int mode, int loop) {

    // チェック
    if (!(fpHCA && (mode == 0 || mode == 8 || mode == 16 || mode == 24 || mode == 32) && loop >= 0))return NULL;

    // 
    FILE *fp1 = (FILE *)fpHCA;
    unsigned int address = ftell(fp1);

    // ヘッダチェック
    stHeader header;
    memset(&header, 0, sizeof(header));
    fread(&header, sizeof(header), 1, fp1);
    if (!CheckFile(&header, sizeof(header)))return NULL;

    // ヘッダ解析
    header.dataOffset = bswap(header.dataOffset);
    unsigned char *data1 = new unsigned char[header.dataOffset];
    if (!data1) { fclose(fp1); return NULL; }
    fseek(fp1, address, SEEK_SET);
    fread(data1, header.dataOffset, 1, fp1);
    if (!Decode(data1, header.dataOffset, 0)) { delete[] data1; return NULL; }

    // WAVEヘッダを書き込み
    struct stWAVEHeader {
        char riff[4];
        unsigned int riffSize;
        char wave[4];
        char fmt[4];
        unsigned int fmtSize;
        unsigned short fmtType;
        unsigned short fmtChannelCount;
        unsigned int fmtSamplingRate;
        unsigned int fmtSamplesPerSec;
        unsigned short fmtSamplingSize;
        unsigned short fmtBitCount;
    }wavRiff = { 'R','I','F','F',0,'W','A','V','E','f','m','t',' ',0x10,0,0,0,0,0,0 };
    struct stWAVEsmpl {
        char smpl[4];
        unsigned int smplSize;
        unsigned int manufacturer;
        unsigned int product;
        unsigned int samplePeriod;
        unsigned int MIDIUnityNote;
        unsigned int MIDIPitchFraction;
        unsigned int SMPTEFormat;
        unsigned int SMPTEOffset;
        unsigned int sampleLoops;
        unsigned int samplerData;
        unsigned int loop_Identifier;
        unsigned int loop_Type;
        unsigned int loop_Start;
        unsigned int loop_End;
        unsigned int loop_Fraction;
        unsigned int loop_PlayCount;
    }wavSmpl = { 's','m','p','l',0x3C,0,0,0,0x3C,0,0,0,1,0x18,0,0,0,0,0,0 };
    struct stWAVEnote {
        char note[4];
        unsigned int noteSize;
        unsigned int dwName;
    }wavNote = { 'n','o','t','e',0,0 };
    struct stWAVEdata {
        char data[4];
        unsigned int dataSize;
    }wavData = { 'd','a','t','a',0 };
    wavRiff.fmtType = (mode>0) ? 1 : 3;
    wavRiff.fmtChannelCount = _channelCount;
    wavRiff.fmtBitCount = (mode>0) ? mode : 32;
    wavRiff.fmtSamplingRate = _samplingRate;
    wavRiff.fmtSamplingSize = wavRiff.fmtBitCount / 8 * wavRiff.fmtChannelCount;
    wavRiff.fmtSamplesPerSec = wavRiff.fmtSamplingRate*wavRiff.fmtSamplingSize;
    if (_loopFlg) {
        wavSmpl.samplePeriod = (unsigned int)(1 / (double)wavRiff.fmtSamplingRate * 1000000000);
        wavSmpl.loop_Start = _loopStart * 0x80 * 8 + _muteFooter;//※計算方法不明
        wavSmpl.loop_End = (_loopEnd + 1) * 0x80 * 8 - 1;//※計算方法不明
        wavSmpl.loop_PlayCount = (_loopCount == 0x80) ? 0 : _loopCount;
    }
    else if (loop) {
        wavSmpl.loop_Start = 0;
        wavSmpl.loop_End = (_blockCount + 1) * 0x80 * 8 - 1;//※計算方法不明
        _loopStart = 0;
        _loopEnd = _blockCount;
    }
    if (_comm_comment) {
        wavNote.noteSize = 4 + _comm_len + 1;
        if (wavNote.noteSize & 3)wavNote.noteSize += 4 - (wavNote.noteSize & 3);
    }
    wavData.dataSize = _blockCount * 0x80 * 8 * wavRiff.fmtSamplingSize + (wavSmpl.loop_End - wavSmpl.loop_Start)*loop;
    wavRiff.riffSize = 0x1C + ((_loopFlg && !loop) ? sizeof(wavSmpl) : 0) + (_comm_comment ? 8 + wavNote.noteSize : 0) + sizeof(wavData) + wavData.dataSize;

    sz = _blockCount * 8 * 128 * _channelCount * 2 + sizeof(wavRiff) + sizeof(wavData);
    char* ptr = new char[sz];
    int seekhead = 0;
    for (int i = 0; i < sizeof(wavRiff); ++i)
    {
        ptr[seekhead++] = ((char*)(&wavRiff))[i];
    }

    //fwrite(&wavRiff, sizeof(wavRiff), 1, fp2);
    /*if (_loopFlg && !loop)fwrite(&wavSmpl, sizeof(wavSmpl), 1, fp2);
    if (_comm_comment) {
    int address = ftell(fp2);
    fwrite(&wavNote, sizeof(wavNote), 1, fp2);
    fputs(_comm_comment, fp2);
    fseek(fp2, address + 8 + wavNote.noteSize, SEEK_SET);
    }*/

    for (int i = 0; i < sizeof(wavData); ++i)
    {
        ptr[seekhead++] = ((char*)(&wavData))[i];
    }

    // デコード
    void *modeFunction = DecodeToMemory_DecodeMode16bit;
    /*switch (mode) {
    case 0:modeFunction = DecodeToWavefile_DecodeModeFloat; break;
    case 8:modeFunction = DecodeToWavefile_DecodeMode8bit; break;
    case 16:modeFunction = DecodeToWavefile_DecodeMode16bit; break;
    case 24:modeFunction = DecodeToWavefile_DecodeMode24bit; break;
    case 32:modeFunction = DecodeToWavefile_DecodeMode32bit; break;
    }*/
    unsigned char *data2 = new unsigned char[_blockSize];
    if (!data2) { delete[] data1; delete[] ptr; return NULL; }
    //if (!loop) {
    if (!DecodeToMemory_Decode(fp1, address + _dataOffset, _blockCount, data2, modeFunction, ptr, seekhead)) { delete[] data2; delete[] data1; delete[] ptr; return NULL; }
    //}
    /*else {
    unsigned int loopBlockOffset = _dataOffset + _loopStart * _blockSize;
    unsigned int loopBlockCount = _loopEnd - _loopStart;
    if (!DecodeToWavefile_Decode(fp1, fp2, address + _dataOffset, _loopEnd, data2, modeFunction)) { delete[] data2; delete[] data1; fclose(fp2); return false; }
    for (int i = 1; i<loop; i++) {
    if (!DecodeToWavefile_Decode(fp1, fp2, address + loopBlockOffset, loopBlockCount, data2, modeFunction)) { delete[] data2; delete[] data1; fclose(fp2); return false; }
    }
    if (!DecodeToWavefile_Decode(fp1, fp2, address + loopBlockOffset, _blockCount - _loopStart, data2, modeFunction)) { delete[] data2; delete[] data1; fclose(fp2); return false; }
    }*/
    delete[] data2;
    delete[] data1;
    // 閉じる

    return ptr;
}
bool clHCA::DecodeToMemory_Decode(void *fp1, unsigned int address, unsigned int count, void *data, void *modeFunction, void* ptr, int& seekhead) {
    float f;
    fseek((FILE *)fp1, address, SEEK_SET);
    for (unsigned int l = 0; l<count; l++, address += _blockSize) {
        fread(data, _blockSize, 1, (FILE *)fp1);
        if (!Decode(data, _blockSize, address))return false;
        for (int i = 0; i<8; i++) {
            for (int j = 0; j<0x80; j++) {
                for (unsigned int k = 0; k<_channelCount; k++) {
                    f = _channel[k].wave[i][j];
                    if (f>1) { f = 1; }
                    else if (f<-1) { f = -1; }
                    ((void(*)(float, void*, int&))modeFunction)(f, ptr, seekhead);
                }
            }
        }
    }
    return true;
}
void clHCA::DecodeToMemory_DecodeMode16bit(float f, void* ptr, int& seekhead) {
    ((short*)ptr)[seekhead/2] = (short)(f * 0x7FFF);
    seekhead += 2;
}
bool clHCA::DecodeToWavefileStream(void *fpHCA, const char *filenameWAV, float volume, int mode, int loop) {

    // チェック
    if (!(fpHCA&&filenameWAV && (mode == 0 || mode == 8 || mode == 16 || mode == 24 || mode == 32) && loop >= 0))return false;

    // 
    FILE *fp1 = (FILE *)fpHCA;
    unsigned int address = ftell(fp1);

    // ヘッダチェック
    stHeader header;
    memset(&header, 0, sizeof(header));
    fread(&header, sizeof(header), 1, fp1);
    if (!CheckFile(&header, sizeof(header)))return false;

    // ヘッダ解析
    header.dataOffset = bswap(header.dataOffset);
    unsigned char *data1 = new unsigned char[header.dataOffset];
    if (!data1) { fclose(fp1); return false; }
    fseek(fp1, address, SEEK_SET);
    fread(data1, header.dataOffset, 1, fp1);
    if (!Decode(data1, header.dataOffset, 0)) { delete[] data1; return false; }

    // WAVEファイルを開く
    FILE *fp2;
    if (fopen_s(&fp2, filenameWAV, "wb")) { delete[] data1; return false; }

    // WAVEヘッダを書き込み
    struct stWAVEHeader {
        char riff[4];
        unsigned int riffSize;
        char wave[4];
        char fmt[4];
        unsigned int fmtSize;
        unsigned short fmtType;
        unsigned short fmtChannelCount;
        unsigned int fmtSamplingRate;
        unsigned int fmtSamplesPerSec;
        unsigned short fmtSamplingSize;
        unsigned short fmtBitCount;
    }wavRiff = { 'R','I','F','F',0,'W','A','V','E','f','m','t',' ',0x10,0,0,0,0,0,0 };
    struct stWAVEsmpl {
        char smpl[4];
        unsigned int smplSize;
        unsigned int manufacturer;
        unsigned int product;
        unsigned int samplePeriod;
        unsigned int MIDIUnityNote;
        unsigned int MIDIPitchFraction;
        unsigned int SMPTEFormat;
        unsigned int SMPTEOffset;
        unsigned int sampleLoops;
        unsigned int samplerData;
        unsigned int loop_Identifier;
        unsigned int loop_Type;
        unsigned int loop_Start;
        unsigned int loop_End;
        unsigned int loop_Fraction;
        unsigned int loop_PlayCount;
    }wavSmpl = { 's','m','p','l',0x3C,0,0,0,0x3C,0,0,0,1,0x18,0,0,0,0,0,0 };
    struct stWAVEnote {
        char note[4];
        unsigned int noteSize;
        unsigned int dwName;
    }wavNote = { 'n','o','t','e',0,0 };
    struct stWAVEdata {
        char data[4];
        unsigned int dataSize;
    }wavData = { 'd','a','t','a',0 };
    wavRiff.fmtType = (mode>0) ? 1 : 3;
    wavRiff.fmtChannelCount = _channelCount;
    wavRiff.fmtBitCount = (mode>0) ? mode : 32;
    wavRiff.fmtSamplingRate = _samplingRate;
    wavRiff.fmtSamplingSize = wavRiff.fmtBitCount / 8 * wavRiff.fmtChannelCount;
    wavRiff.fmtSamplesPerSec = wavRiff.fmtSamplingRate*wavRiff.fmtSamplingSize;
    if (_loopFlg) {
        wavSmpl.samplePeriod = (unsigned int)(1 / (double)wavRiff.fmtSamplingRate * 1000000000);
        wavSmpl.loop_Start = _loopStart * 0x80 * 8 + _muteFooter;//※計算方法不明
        wavSmpl.loop_End = (_loopEnd + 1) * 0x80 * 8 - 1;//※計算方法不明
        wavSmpl.loop_PlayCount = (_loopCount == 0x80) ? 0 : _loopCount;
    }
    else if (loop) {
        wavSmpl.loop_Start = 0;
        wavSmpl.loop_End = (_blockCount + 1) * 0x80 * 8 - 1;//※計算方法不明
        _loopStart = 0;
        _loopEnd = _blockCount;
    }
    if (_comm_comment) {
        wavNote.noteSize = 4 + _comm_len + 1;
        if (wavNote.noteSize & 3)wavNote.noteSize += 4 - (wavNote.noteSize & 3);
    }
    wavData.dataSize = _blockCount * 0x80 * 8 * wavRiff.fmtSamplingSize + (wavSmpl.loop_End - wavSmpl.loop_Start)*loop;
    wavRiff.riffSize = 0x1C + ((_loopFlg && !loop) ? sizeof(wavSmpl) : 0) + (_comm_comment ? 8 + wavNote.noteSize : 0) + sizeof(wavData) + wavData.dataSize;
    fwrite(&wavRiff, sizeof(wavRiff), 1, fp2);
    if (_loopFlg && !loop)fwrite(&wavSmpl, sizeof(wavSmpl), 1, fp2);
    if (_comm_comment) {
        int address = ftell(fp2);
        fwrite(&wavNote, sizeof(wavNote), 1, fp2);
        fputs(_comm_comment, fp2);
        fseek(fp2, address + 8 + wavNote.noteSize, SEEK_SET);
    }
    fwrite(&wavData, sizeof(wavData), 1, fp2);

    // 相対ボリュームを調節
    _rva_volume *= volume;

    // デコード
    void *modeFunction = DecodeToWavefile_DecodeMode16bit;
    switch (mode) {
    case 0:modeFunction = DecodeToWavefile_DecodeModeFloat; break;
    case 8:modeFunction = DecodeToWavefile_DecodeMode8bit; break;
    case 16:modeFunction = DecodeToWavefile_DecodeMode16bit; break;
    case 24:modeFunction = DecodeToWavefile_DecodeMode24bit; break;
    case 32:modeFunction = DecodeToWavefile_DecodeMode32bit; break;
    }
    unsigned char *data2 = new unsigned char[_blockSize];
    if (!data2) { delete[] data1; fclose(fp2); return false; }
    if (!loop) {
        if (!DecodeToWavefile_Decode(fp1, fp2, address + _dataOffset, _blockCount, data2, modeFunction)) { delete[] data2; delete[] data1; fclose(fp2); return false; }
    }
    else {
        unsigned int loopBlockOffset = _dataOffset + _loopStart * _blockSize;
        unsigned int loopBlockCount = _loopEnd - _loopStart;
        if (!DecodeToWavefile_Decode(fp1, fp2, address + _dataOffset, _loopEnd, data2, modeFunction)) { delete[] data2; delete[] data1; fclose(fp2); return false; }
        for (int i = 1; i<loop; i++) {
            if (!DecodeToWavefile_Decode(fp1, fp2, address + loopBlockOffset, loopBlockCount, data2, modeFunction)) { delete[] data2; delete[] data1; fclose(fp2); return false; }
        }
        if (!DecodeToWavefile_Decode(fp1, fp2, address + loopBlockOffset, _blockCount - _loopStart, data2, modeFunction)) { delete[] data2; delete[] data1; fclose(fp2); return false; }
    }
    delete[] data2;
    delete[] data1;
    // 閉じる
    fclose(fp2);

    return true;
}
unsigned int clHCA::get_channelCount() const
{
    return _channelCount;
}
unsigned int clHCA::get_blockCount() const
{
    return _blockCount;
}
unsigned int clHCA::get_blockSize() const
{
    return _blockSize;
}
bool clHCA::DecodeToWavefile_Decode(void *fp1, void *fp2, unsigned int address, unsigned int count, void *data, void *modeFunction) {
    float f;
    fseek((FILE *)fp1, address, SEEK_SET);
    for (unsigned int l = 0; l<count; l++, address += _blockSize) {
        fread(data, _blockSize, 1, (FILE *)fp1);
        if (!Decode(data, _blockSize, address))return false;
        for (int i = 0; i<8; i++) {
            for (int j = 0; j<0x80; j++) {
                for (unsigned int k = 0; k<_channelCount; k++) {
                    f = _channel[k].wave[i][j] * _rva_volume;
                    if (f>1) { f = 1; }
                    else if (f<-1) { f = -1; }
                    ((void(*)(float, void *))modeFunction)(f, fp2);
                }
            }
        }
    }
    return true;
}
void clHCA::DecodeToWavefile_DecodeModeFloat(float f, void *fp) { fwrite(&f, sizeof(f), 1, (FILE *)fp); }
void clHCA::DecodeToWavefile_DecodeMode8bit(float f, void *fp) { int v = (int)(f * 0x7F) + 0x80; fwrite(&v, 1, 1, (FILE *)fp); }
void clHCA::DecodeToWavefile_DecodeMode16bit(float f, void *fp) { int v = (int)(f * 0x7FFF); fwrite(&v, 2, 1, (FILE *)fp); }
void clHCA::DecodeToWavefile_DecodeMode24bit(float f, void *fp) { int v = (int)(f * 0x7FFFFF); fwrite(&v, 3, 1, (FILE *)fp); }
void clHCA::DecodeToWavefile_DecodeMode32bit(float f, void *fp) { int v = (int)(f * 0x7FFFFFFF); fwrite(&v, 4, 1, (FILE *)fp); }

//--------------------------------------------------
// エンコードしてHCAファイルに保存
//--------------------------------------------------
/*bool clHCA::EncodeFromWavefile(const char *filenameWAV,const char *filenameHCA,float volume){

// チェック
if(!(filenameWAV))return false;

// WAVファイルを開く
FILE *fp;
if(fopen_s(&fp,filenameWAV,"rb"))return false;

// 保存
if(!EncodeFromWavefileStream(fp,filenameHCA,volume)){fclose(fp);return false;}

// 閉じる
fclose(fp);

return true;
}
bool clHCA::EncodeFromWavefileStream(void *fpWAV,const char *filenameHCA,float volume){

// チェック
if(!(fpWAV&&filenameHCA))return false;

//
FILE *fp1=(FILE *)fpWAV;
unsigned int address=ftell(fp1);

// ヘッダチェック
struct stWAVEHeader{
unsigned int riff;
unsigned int riffSize;
unsigned int wave;
unsigned int fmt;
unsigned int fmtSize;
unsigned short fmtType;
unsigned short fmtChannelCount;
unsigned int fmtSamplingRate;
unsigned int fmtSamplesPerSec;
unsigned short fmtSamplingSize;
unsigned short fmtBitCount;
}header;
memset(&header,0,sizeof(header));
fread(&header,sizeof(header),1,fp1);
if(!(header.riff==0x46464952&&header.wave==0x45564157&&header.fmt==0x20746D66&&(header.fmtType==1||header.fmtType==3)))return false;

//@@@@@@@@@@@@@@@@@@@@@@@

return true;
}*/

//--------------------------------------------------
// ATH
//--------------------------------------------------
clHCA::clATH::clATH() { Init0(); }
bool clHCA::clATH::Init(int type, unsigned int key) {
    switch (type) {
    case 0:Init0(); break;
    case 1:Init1(key); break;
    default:return false;
    }
    return true;
}
unsigned char *clHCA::clATH::GetTable(void) {
    return _table;
}
void clHCA::clATH::Init0(void) {
    memset(_table, 0, sizeof(_table));
}
void clHCA::clATH::Init1(unsigned int key) {
    static unsigned char list[] = {
        0x78,0x5F,0x56,0x51,0x4E,0x4C,0x4B,0x49,0x48,0x48,0x47,0x46,0x46,0x45,0x45,0x45,
        0x44,0x44,0x44,0x44,0x43,0x43,0x43,0x43,0x43,0x43,0x42,0x42,0x42,0x42,0x42,0x42,
        0x42,0x42,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x40,0x40,0x40,0x40,
        0x40,0x40,0x40,0x40,0x40,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
        0x3F,0x3F,0x3F,0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,
        0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,
        0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,
        0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,
        0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3F,
        0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
        0x3F,0x3F,0x3F,0x3F,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
        0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
        0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
        0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,
        0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x43,0x43,0x43,
        0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x44,0x44,
        0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x45,0x45,0x45,0x45,
        0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,
        0x46,0x46,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x48,0x48,0x48,0x48,
        0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,0x49,0x49,0x49,0x49,0x4A,0x4A,0x4A,0x4A,
        0x4A,0x4A,0x4A,0x4A,0x4B,0x4B,0x4B,0x4B,0x4B,0x4B,0x4B,0x4C,0x4C,0x4C,0x4C,0x4C,
        0x4C,0x4D,0x4D,0x4D,0x4D,0x4D,0x4D,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4F,0x4F,0x4F,
        0x4F,0x4F,0x4F,0x50,0x50,0x50,0x50,0x50,0x51,0x51,0x51,0x51,0x51,0x52,0x52,0x52,
        0x52,0x52,0x53,0x53,0x53,0x53,0x54,0x54,0x54,0x54,0x54,0x55,0x55,0x55,0x55,0x56,
        0x56,0x56,0x56,0x57,0x57,0x57,0x57,0x57,0x58,0x58,0x58,0x59,0x59,0x59,0x59,0x5A,
        0x5A,0x5A,0x5A,0x5B,0x5B,0x5B,0x5B,0x5C,0x5C,0x5C,0x5D,0x5D,0x5D,0x5D,0x5E,0x5E,
        0x5E,0x5F,0x5F,0x5F,0x60,0x60,0x60,0x61,0x61,0x61,0x61,0x62,0x62,0x62,0x63,0x63,
        0x63,0x64,0x64,0x64,0x65,0x65,0x66,0x66,0x66,0x67,0x67,0x67,0x68,0x68,0x68,0x69,
        0x69,0x6A,0x6A,0x6A,0x6B,0x6B,0x6B,0x6C,0x6C,0x6D,0x6D,0x6D,0x6E,0x6E,0x6F,0x6F,
        0x70,0x70,0x70,0x71,0x71,0x72,0x72,0x73,0x73,0x73,0x74,0x74,0x75,0x75,0x76,0x76,
        0x77,0x77,0x78,0x78,0x78,0x79,0x79,0x7A,0x7A,0x7B,0x7B,0x7C,0x7C,0x7D,0x7D,0x7E,
        0x7E,0x7F,0x7F,0x80,0x80,0x81,0x81,0x82,0x83,0x83,0x84,0x84,0x85,0x85,0x86,0x86,
        0x87,0x88,0x88,0x89,0x89,0x8A,0x8A,0x8B,0x8C,0x8C,0x8D,0x8D,0x8E,0x8F,0x8F,0x90,
        0x90,0x91,0x92,0x92,0x93,0x94,0x94,0x95,0x95,0x96,0x97,0x97,0x98,0x99,0x99,0x9A,
        0x9B,0x9B,0x9C,0x9D,0x9D,0x9E,0x9F,0xA0,0xA0,0xA1,0xA2,0xA2,0xA3,0xA4,0xA5,0xA5,
        0xA6,0xA7,0xA7,0xA8,0xA9,0xAA,0xAA,0xAB,0xAC,0xAD,0xAE,0xAE,0xAF,0xB0,0xB1,0xB1,
        0xB2,0xB3,0xB4,0xB5,0xB6,0xB6,0xB7,0xB8,0xB9,0xBA,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
        0xC0,0xC1,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xC9,0xCA,0xCB,0xCC,0xCD,
        0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,
        0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xED,0xEE,
        0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFF,0xFF,
    };
    for (unsigned int i = 0, v = 0; i<0x80; i++, v += key) {
        unsigned int index = v >> 13;
        if (index >= 0x28E) {
            memset(&_table[i], 0xFF, 0x80 - i);
            break;
        }
        _table[i] = list[index];
    }
}

//--------------------------------------------------
// 暗号化テーブル
//--------------------------------------------------
clHCA::clCipher::clCipher() { Init0(); }
bool clHCA::clCipher::Init(int type, unsigned int key1, unsigned int key2) {
    if (!(key1 | key2))type = 0;
    switch (type) {
    case 0:Init0(); break;
    case 1:Init1(); break;
    case 56:Init56(key1, key2); break;
    default:return false;
    }
    return true;
}
void clHCA::clCipher::Mask(void *data, int size) {
    for (unsigned char *d = (unsigned char *)data; size>0; d++, size--)*d = _table[*d];
}
void clHCA::clCipher::Init0(void) {
    for (int i = 0; i<0x100; i++)_table[i] = i;
}
void clHCA::clCipher::Init1(void) {
    for (int i = 1, v = 0; i<0xFF; i++) {
        v = (v * 13 + 11) & 0xFF;
        if (v == 0 || v == 0xFF)v = (v * 13 + 11) & 0xFF;
        _table[i] = v;
    }
    _table[0] = 0;
    _table[0xFF] = 0xFF;
}
void clHCA::clCipher::Init56(unsigned int key1, unsigned int key2) {

    // テーブル1を生成
    unsigned char t1[8];
    if (!key1)key2--;
    key1--;
    for (int i = 0; i<7; i++) {
        t1[i] = key1;
        key1 = (key1 >> 8) | (key2 << 24);
        key2 >>= 8;
    }

    // テーブル2
    unsigned char t2[0x10] = {
        t1[1],t1[1] ^ t1[6],
        t1[2] ^ t1[3],t1[2],
        t1[2] ^ t1[1],t1[3] ^ t1[4],
        t1[3],t1[3] ^ t1[2],
        t1[4] ^ t1[5],t1[4],
        t1[4] ^ t1[3],t1[5] ^ t1[6],
        t1[5],t1[5] ^ t1[4],
        t1[6] ^ t1[1],t1[6],
    };

    // テーブル3
    unsigned char t3[0x100], t31[0x10], t32[0x10], *t = t3;
    Init56_CreateTable(t31, t1[0]);
    for (int i = 0; i<0x10; i++) {
        Init56_CreateTable(t32, t2[i]);
        unsigned char v = t31[i] << 4;
        for (int j = 0; j<0x10; j++) {
            *(t++) = v | t32[j];
        }
    }

    // CIPHテーブル
    t = &_table[1];
    for (int i = 0, v = 0; i<0x100; i++) {
        v = (v + 0x11) & 0xFF;
        unsigned char a = t3[v];
        if (a != 0 && a != 0xFF)*(t++) = a;
    }
    _table[0] = 0;
    _table[0xFF] = 0xFF;

}
void clHCA::clCipher::Init56_CreateTable(unsigned char *r, unsigned char key) {
    int mul = ((key & 1) << 3) | 5;
    int add = (key & 0xE) | 1;
    key >>= 4;
    for (int i = 0; i<0x10; i++) {
        key = (key*mul + add) & 0xF;
        *(r++) = key;
    }
}

//--------------------------------------------------
// データ
//--------------------------------------------------
clHCA::clData::clData(void *data, int size) :_data((unsigned char *)data), _size(size * 8 - 16), _bit(0) {}
int clHCA::clData::CheckBit(int bitSize) {
    int v = 0;
    if (_bit + bitSize <= _size) {
        static int mask[] = { 0xFFFFFF,0x7FFFFF,0x3FFFFF,0x1FFFFF,0x0FFFFF,0x07FFFF,0x03FFFF,0x01FFFF };
        unsigned char *data = &_data[_bit >> 3];
        v = data[0]; v = (v << 8) | data[1]; v = (v << 8) | data[2];
        v &= mask[_bit & 7];
        v >>= 24 - (_bit & 7) - bitSize;
    }
    return v;
}
int clHCA::clData::GetBit(int bitSize) {
    int v = CheckBit(bitSize);
    _bit += bitSize;
    return v;
}
void clHCA::clData::AddBit(int bitSize) {
    _bit += bitSize;
}

//--------------------------------------------------
// デコード
//--------------------------------------------------
bool clHCA::Decode(void *data, unsigned int size, unsigned int address) {

    // チェック
    if (!(data))return false;

    // ヘッダ
    if (address == 0) {
        unsigned char *s = (unsigned char *)data;

        // サイズチェック
        if (size<sizeof(stHeader))return false;

        // HCA
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00414348) {
            stHeader *hca = (stHeader *)s; s += sizeof(stHeader);
            _version = bswap(hca->version);
            _dataOffset = bswap(hca->dataOffset);
            //if(!(_version<=0x200&&_version>0x101))return false; // バージョンチェック(無効)
            if (size<_dataOffset)return false;
            //if(CheckSum(hca,_dataOffset))return false; // ヘッダの破損チェック(ヘッダ改変を有効にするため破損チェック無効)
        }
        else {
            return false;
        }

        // fmt
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00746D66) {
            stFormat *fmt = (stFormat *)s; s += sizeof(stFormat);
            _channelCount = fmt->channelCount;
            _samplingRate = bswap(fmt->samplingRate << 8);
            _blockCount = bswap(fmt->blockCount);
            _muteHeader = bswap(fmt->muteHeader);
            _muteFooter = bswap(fmt->muteFooter);
            if (!(_channelCount >= 1 && _channelCount <= 16))return false;
            if (!(_samplingRate >= 1 && _samplingRate <= 0x7FFFFF))return false;
        }
        else {
            return false;
        }

        // comp
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706D6F63) {
            stCompress *comp = (stCompress *)s; s += sizeof(stCompress);
            _blockSize = bswap(comp->blockSize);
            _comp_r01 = comp->r01;
            _comp_r02 = comp->r02;
            _comp_r03 = comp->r03;
            _comp_r04 = comp->r04;
            _comp_r05 = comp->r05;
            _comp_r06 = comp->r06;
            _comp_r07 = comp->r07;
            _comp_r08 = comp->r08;
            if (!((_blockSize >= 8 && _blockSize <= 0xFFFF) || (_blockSize == 0)))return false;
            if (!(_comp_r01 >= 0 && _comp_r01 <= _comp_r02 && _comp_r02 <= 0x1F))return false;
        }

        // dec
        else if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00636564) {
            stDecode *dec = (stDecode *)s; s += sizeof(stDecode);
            _blockSize = bswap(dec->blockSize);
            _comp_r01 = dec->r01;
            _comp_r02 = dec->r02;
            _comp_r03 = dec->r04;
            _comp_r04 = dec->r03;
            _comp_r05 = dec->count1 + 1;
            _comp_r06 = ((dec->enableCount2) ? dec->count2 : dec->count1) + 1;
            _comp_r07 = _comp_r05 - _comp_r06;
            _comp_r08 = 0;
            if (!((_blockSize >= 8 && _blockSize <= 0xFFFF) || (_blockSize == 0)))return false;
            if (!(_comp_r01 >= 0 && _comp_r01 <= _comp_r02 && _comp_r02 <= 0x1F))return false;
            if (!_comp_r03)_comp_r03 = 1;
        }
        else {
            return false;
        }

        // vbr
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00726276) {
            stVBR *vbr = (stVBR *)s; s += sizeof(stVBR);
            _vbr_r01 = bswap(vbr->r01);
            _vbr_r02 = bswap(vbr->r02);
            if (!(_blockSize == 0 && _vbr_r01 >= 0 && _vbr_r01 <= 0x1FF))return false;
        }
        else {
            _vbr_r01 = 0;
            _vbr_r02 = 0;
        }

        // ath
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00687461) {
            stATH *ath = (stATH *)s; s += 6;//s+=sizeof(stATH);
            _ath_type = ath->type;
        }
        else {
            _ath_type = (_version<0x200) ? 1 : 0;//v1.3ではデフォルト値が1になってたが、v2.0からATHテーブルが廃止されてるみたいなので0に
        }

        // loop
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706F6F6C) {
            stLoop *loop = (stLoop *)s; s += sizeof(stLoop);
            _loopStart = bswap(loop->start);
            _loopEnd = bswap(loop->end);
            _loopCount = bswap(loop->count);
            _loop_r01 = bswap(loop->r01);
            _loopFlg = true;
            if (!(_loopStart >= 0 && _loopStart <= _loopEnd && _loopEnd<_blockCount))return false;
        }
        else {
            _loopStart = 0;
            _loopEnd = 0;
            _loopCount = 0;
            _loop_r01 = 0x400;
            _loopFlg = false;
        }

        // ciph
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x68706963) {
            stCipher *ciph = (stCipher *)s; s += 6;//s+=sizeof(stCipher);
            _ciph_type = bswap(ciph->type);
            if (!(_ciph_type == 0 || _ciph_type == 1 || _ciph_type == 0x38))return false;
        }
        else {
            _ciph_type = 0;
        }

        // rva
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00617672) {
            stRVA *rva = (stRVA *)s; s += sizeof(stRVA);
            _rva_volume = bswap(rva->volume);
        }
        else {
            _rva_volume = 1;
        }

        // comm
        if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x6D6D6F63) {
            stComment *comm = (stComment *)s; s += 5;//s+=sizeof(stComment);
            _comm_len = comm->len;
            _comm_comment = (char *)s;
        }
        else {
            _comm_len = 0;
            _comm_comment = NULL;
        }

        // 初期化
        if (!_ath.Init(_ath_type, _samplingRate))return false;
        if (!_cipher.Init(_ciph_type, _ciph_key1, _ciph_key2))return false;

        // 値チェック(ヘッダの改変ミスによるエラーを回避するため)
        if (!_comp_r03)_comp_r03 = 1;//0での除算を防ぐため

                                     // デコード準備
        memset(_channel, 0, sizeof(_channel));
        if (!(_comp_r01 == 1 && _comp_r02 == 15))return false;
        _comp_r09 = ceil2(_comp_r05 - (_comp_r06 + _comp_r07), _comp_r08);
        char r[0x10]; memset(r, 0, sizeof(r));
        unsigned int b = _channelCount / _comp_r03;
        if (_comp_r07&&b>1) {
            char *c = r;
            for (unsigned int i = 0; i<_comp_r03; i++, c += b) {
                switch (b) {
                case 2:c[0] = 1; c[1] = 2; break;
                case 3:c[0] = 1; c[1] = 2; break;
                case 4:c[0] = 1; c[1] = 2; if (_comp_r04 == 0) { c[2] = 1; c[3] = 2; }break;
                case 5:c[0] = 1; c[1] = 2; if (_comp_r04 <= 2) { c[3] = 1; c[4] = 2; }break;
                case 6:c[0] = 1; c[1] = 2; c[4] = 1; c[5] = 2; break;
                case 7:c[0] = 1; c[1] = 2; c[4] = 1; c[5] = 2; break;
                case 8:c[0] = 1; c[1] = 2; c[4] = 1; c[5] = 2; c[6] = 1; c[7] = 2; break;
                }
            }
        }
        for (unsigned int i = 0; i<_channelCount; i++) {
            _channel[i].type = r[i];
            _channel[i].value3 = &_channel[i].value[_comp_r06 + _comp_r07];
            _channel[i].count = _comp_r06 + ((r[i] != 2) ? _comp_r07 : 0);
        }

    }

    // ブロックデータ
    else if (address >= _dataOffset) {
        if (size<_blockSize)return false;
        if (CheckSum(data, _blockSize))return false;
        //        if(((unsigned char *)data)[_blockSize-2]==0x5E)_asm int 3
        _cipher.Mask(data, _blockSize);
        clData d(data, _blockSize);
        int magic = d.GetBit(16);//0xFFFF固定
        if (magic == 0xFFFF) {
            int a = (d.GetBit(9) << 8) - d.GetBit(7);
            for (unsigned int i = 0; i<_channelCount; i++)_channel[i].Decode1(&d, _comp_r09, a, _ath.GetTable());
            for (int i = 0; i<8; i++) {
                for (unsigned int j = 0; j<_channelCount; j++)_channel[j].Decode2(&d);
                for (unsigned int j = 0; j<_channelCount; j++)_channel[j].Decode3(_comp_r09, _comp_r08, _comp_r07 + _comp_r06, _comp_r05);
                for (unsigned int j = 0; j<_channelCount - 1; j++)_channel[j].Decode4(i, _comp_r05 - _comp_r06, _comp_r06, _comp_r07);
                for (unsigned int j = 0; j<_channelCount; j++)_channel[j].Decode5(i);
            }
        }
    }

    return true;
}

bool clHCA::PrepDecode(stChannel* channels, unsigned int numthreads)
{
    memset(channels, 0, sizeof(stChannel) * numthreads * _channelCount);
    if (!(_comp_r01 == 1 && _comp_r02 == 15))return false;
    _comp_r09 = ceil2(_comp_r05 - (_comp_r06 + _comp_r07), _comp_r08);
    char r[0x10]; memset(r, 0, sizeof(r));
    unsigned int b = _channelCount / _comp_r03;
    if (_comp_r07&&b>1) {
        char *c = r;
        for (unsigned int i = 0; i<_comp_r03; i++, c += b) {
            switch (b) {
            case 2:c[0] = 1; c[1] = 2; break;
            case 3:c[0] = 1; c[1] = 2; break;
            case 4:c[0] = 1; c[1] = 2; if (_comp_r04 == 0) { c[2] = 1; c[3] = 2; }break;
            case 5:c[0] = 1; c[1] = 2; if (_comp_r04 <= 2) { c[3] = 1; c[4] = 2; }break;
            case 6:c[0] = 1; c[1] = 2; c[4] = 1; c[5] = 2; break;
            case 7:c[0] = 1; c[1] = 2; c[4] = 1; c[5] = 2; break;
            case 8:c[0] = 1; c[1] = 2; c[4] = 1; c[5] = 2; c[6] = 1; c[7] = 2; break;
            }
        }
    }
    for (unsigned int i = 0; i < _channelCount * numthreads; i++)
    {
        channels[i].type = r[i % _channelCount];
        channels[i].value3 = &channels[i].value[_comp_r06 + _comp_r07];
        channels[i].count = _comp_r06 + ((r[i % _channelCount] != 2) ? _comp_r07 : 0);
    }
    return true;
}

//--------------------------------------------------
// デコード第一段階
//   ベースデータの読み込み
//--------------------------------------------------
void clHCA::stChannel::Decode1(clData *data, unsigned int a, int b, unsigned char *ath) {
    static unsigned char scalelist[] = {
        // v2.0
        0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0D,0x0D,
        0x0D,0x0D,0x0D,0x0D,0x0C,0x0C,0x0C,0x0C,
        0x0C,0x0C,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
        0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x09,
        0x09,0x09,0x09,0x09,0x09,0x08,0x08,0x08,
        0x08,0x08,0x08,0x07,0x06,0x06,0x05,0x04,
        0x04,0x04,0x03,0x03,0x03,0x02,0x02,0x02,
        0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        // v1.3
        //0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0D,0x0D,
        //0x0D,0x0D,0x0D,0x0D,0x0C,0x0C,0x0C,0x0C,
        //0x0C,0x0C,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
        //0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x09,
        //0x09,0x09,0x09,0x09,0x09,0x08,0x08,0x08,
        //0x08,0x08,0x08,0x07,0x06,0x06,0x05,0x04,
        //0x04,0x04,0x03,0x03,0x03,0x02,0x02,0x02,
        //0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
    };
    static unsigned int valueInt[] = {
        0x342A8D26,0x34633F89,0x3497657D,0x34C9B9BE,0x35066491,0x353311C4,0x356E9910,0x359EF532,
        0x35D3CCF1,0x360D1ADF,0x363C034A,0x367A83B3,0x36A6E595,0x36DE60F5,0x371426FF,0x3745672A,
        0x37838359,0x37AF3B79,0x37E97C38,0x381B8D3A,0x384F4319,0x388A14D5,0x38B7FBF0,0x38F5257D,
        0x3923520F,0x39599D16,0x3990FA4D,0x39C12C4D,0x3A00B1ED,0x3A2B7A3A,0x3A647B6D,0x3A9837F0,
        0x3ACAD226,0x3B071F62,0x3B340AAF,0x3B6FE4BA,0x3B9FD228,0x3BD4F35B,0x3C0DDF04,0x3C3D08A4,
        0x3C7BDFED,0x3CA7CD94,0x3CDF9613,0x3D14F4F0,0x3D467991,0x3D843A29,0x3DB02F0E,0x3DEAC0C7,
        0x3E1C6573,0x3E506334,0x3E8AD4C6,0x3EB8FBAF,0x3EF67A41,0x3F243516,0x3F5ACB94,0x3F91C3D3,
        0x3FC238D2,0x400164D2,0x402C6897,0x4065B907,0x40990B88,0x40CBEC15,0x4107DB35,0x413504F3,
    };
    static unsigned int scaleInt[] = {
        0x00000000,0x3F2AAAAB,0x3ECCCCCD,0x3E924925,0x3E638E39,0x3E3A2E8C,0x3E1D89D9,0x3E088889,
        0x3D842108,0x3D020821,0x3C810204,0x3C008081,0x3B804020,0x3B002008,0x3A801002,0x3A000801,
    };
    static float *valueFloat = (float *)valueInt;
    static float *scaleFloat = (float *)scaleInt;
    int v = data->GetBit(3);
    if (v >= 6) {
        for (unsigned int i = 0; i<count; i++)value[i] = data->GetBit(6);
    }
    else if (v) {
        int v1 = data->GetBit(6), v2 = (1 << v) - 1, v3 = v2 >> 1, v4;
        value[0] = v1;
        for (unsigned int i = 1; i<count; i++) {
            v4 = data->GetBit(v);
            if (v4 != v2) { v1 += v4 - v3; }
            else { v1 = data->GetBit(6); }
            value[i] = v1;
        }
    }
    else {
        memset(value, 0, 0x80);
    }
    if (type == 2) {
        v = data->CheckBit(4); value2[0] = v;
        if (v<15)for (int i = 0; i<8; i++)value2[i] = data->GetBit(4);
    }
    else {
        for (unsigned int i = 0; i<a; i++)value3[i] = data->GetBit(6);
    }
    for (unsigned int i = 0; i<count; i++) {
        v = value[i];
        if (v) {
            v = ath[i] + ((b + i) >> 8) - ((v * 5) / 2) + 1;
            if (v<0)v = 15;
            else if (v >= 0x39)v = 1;
            else v = scalelist[v];
        }
        scale[i] = v;
    }
    memset(&scale[count], 0, 0x80 - count);
    for (unsigned int i = 0; i<count; i++)base[i] = valueFloat[value[i]] * scaleFloat[scale[i]];
}

//--------------------------------------------------
// デコード第二段階
//   ブロックデータの読み込み
//--------------------------------------------------
void clHCA::stChannel::Decode2(clData *data) {
    static char list1[] = {
        0,2,3,3,4,4,4,4,5,6,7,8,9,10,11,12,
    };
    static char list2[] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,2,2,0,0,0,0,0,0,0,0,0,0,0,0,
        2,2,2,2,2,2,3,3,0,0,0,0,0,0,0,0,
        2,2,3,3,3,3,3,3,0,0,0,0,0,0,0,0,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,
        3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,
        3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,
        3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    };
    static float list3[] = {
        +0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,-1,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,+1,-1,-1,+2,-2,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,-1,+2,-2,+3,-3,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,+1,-1,-1,+2,+2,-2,-2,+3,+3,-3,-3,+4,-4,
        +0,+0,+1,+1,-1,-1,+2,+2,-2,-2,+3,-3,+4,-4,+5,-5,
        +0,+0,+1,+1,-1,-1,+2,-2,+3,-3,+4,-4,+5,-5,+6,-6,
        +0,+0,+1,-1,+2,-2,+3,-3,+4,-4,+5,-5,+6,-6,+7,-7,
    };
    for (unsigned int i = 0; i<count; i++) {
        float f;
        int s = scale[i];
        int bitSize = list1[s];
        int v = data->GetBit(bitSize);
        if (s<8) {
            v += s << 4;
            data->AddBit(list2[v] - bitSize);
            f = list3[v];
        }
        else {
            v = (1 - ((v & 1) << 1))*(v / 2);
            if (!v)data->AddBit(-1);
            f = (float)v;
        }
        block[i] = base[i] * f;
    }
    memset(&block[count], 0, sizeof(float)*(0x80 - count));
}

//--------------------------------------------------
// デコード第三段階
//   ブロックデータ修正その１ ※v2.0から追加
//--------------------------------------------------
void clHCA::stChannel::Decode3(unsigned int a, unsigned int b, unsigned int c, unsigned int d) {
    if (type != 2 && b>0) {
        static unsigned int listInt[2][0x40] = {
            {
                0x00000000,0x00000000,0x32A0B051,0x32D61B5E,0x330EA43A,0x333E0F68,0x337D3E0C,0x33A8B6D5,
                0x33E0CCDF,0x3415C3FF,0x34478D75,0x3484F1F6,0x34B123F6,0x34EC0719,0x351D3EDA,0x355184DF,
                0x358B95C2,0x35B9FCD2,0x35F7D0DF,0x36251958,0x365BFBB8,0x36928E72,0x36C346CD,0x370218AF,
                0x372D583F,0x3766F85B,0x3799E046,0x37CD078C,0x3808980F,0x38360094,0x38728177,0x38A18FAF,
                0x38D744FD,0x390F6A81,0x393F179A,0x397E9E11,0x39A9A15B,0x39E2055B,0x3A16942D,0x3A48A2D8,
                0x3A85AAC3,0x3AB21A32,0x3AED4F30,0x3B1E196E,0x3B52A81E,0x3B8C57CA,0x3BBAFF5B,0x3BF9295A,
                0x3C25FED7,0x3C5D2D82,0x3C935A2B,0x3CC4563F,0x3D02CD87,0x3D2E4934,0x3D68396A,0x3D9AB62B,
                0x3DCE248C,0x3E0955EE,0x3E36FD92,0x3E73D290,0x3EA27043,0x3ED87039,0x3F1031DC,0x3F40213B,
            },{
                0x3F800000,0x3FAA8D26,0x3FE33F89,0x4017657D,0x4049B9BE,0x40866491,0x40B311C4,0x40EE9910,
                0x411EF532,0x4153CCF1,0x418D1ADF,0x41BC034A,0x41FA83B3,0x4226E595,0x425E60F5,0x429426FF,
                0x42C5672A,0x43038359,0x432F3B79,0x43697C38,0x439B8D3A,0x43CF4319,0x440A14D5,0x4437FBF0,
                0x4475257D,0x44A3520F,0x44D99D16,0x4510FA4D,0x45412C4D,0x4580B1ED,0x45AB7A3A,0x45E47B6D,
                0x461837F0,0x464AD226,0x46871F62,0x46B40AAF,0x46EFE4BA,0x471FD228,0x4754F35B,0x478DDF04,
                0x47BD08A4,0x47FBDFED,0x4827CD94,0x485F9613,0x4894F4F0,0x48C67991,0x49043A29,0x49302F0E,
                0x496AC0C7,0x499C6573,0x49D06334,0x4A0AD4C6,0x4A38FBAF,0x4A767A41,0x4AA43516,0x4ADACB94,
                0x4B11C3D3,0x4B4238D2,0x4B8164D2,0x4BAC6897,0x4BE5B907,0x4C190B88,0x4C4BEC15,0x00000000,
            }
        };
        static float *listFloat = (float *)listInt[1];
        for (unsigned int i = 0; i<a; i++) {
            for (unsigned int j = 0, k = c, l = c - 1; j<b&&k<d; j++, l--) {
                block[k++] = listFloat[value3[i] - value[l]] * block[l];
            }
        }
        block[0x80 - 1] = 0;
    }
}

//--------------------------------------------------
// デコード第四段階
//   ブロックデータ修正その２
//--------------------------------------------------
void clHCA::stChannel::Decode4(int index, unsigned int a, unsigned int b, unsigned int c) {
    if (type == 1 && c) {
        static unsigned int listInt[] = {
            0x40000000,0x3FEDB6DB,0x3FDB6DB7,0x3FC92492,0x3FB6DB6E,0x3FA49249,0x3F924925,0x3F800000,
            0x3F5B6DB7,0x3F36DB6E,0x3F124925,0x3EDB6DB7,0x3E924925,0x3E124925,0x00000000,0x00000000,
        };
        float f1 = ((float *)listInt)[this[1].value2[index]];
        float f2 = f1 - 2.0f;
        float *s = &block[b];
        float *d = &this[1].block[b];
        for (unsigned int i = 0; i<a; i++) {
            *(d++) = *s*f2;
            *(s++) = *s*f1;
        }
    }
}

//--------------------------------------------------
// デコード第五段階
//   波形データを生成
//--------------------------------------------------
void clHCA::stChannel::Decode5(int index) {
    static unsigned int list1Int[7][0x40] = {
        {
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
        },{
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
        },{
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
        },{
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
        },{
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
        },{
            0x3F7FFB11,0x3F7FD397,0x3F7F84AB,0x3F7F0E58,0x3F7E70B0,0x3F7DABCC,0x3F7CBFC9,0x3F7BACCD,
            0x3F7A7302,0x3F791298,0x3F778BC5,0x3F75DEC6,0x3F740BDD,0x3F721352,0x3F6FF573,0x3F6DB293,
            0x3F6B4B0C,0x3F68BF3C,0x3F660F88,0x3F633C5A,0x3F604621,0x3F5D2D53,0x3F59F26A,0x3F5695E5,
            0x3F531849,0x3F4F7A1F,0x3F4BBBF8,0x3F47DE65,0x3F43E200,0x3F3FC767,0x3F3B8F3B,0x3F373A23,
            0x3F7FFB11,0x3F7FD397,0x3F7F84AB,0x3F7F0E58,0x3F7E70B0,0x3F7DABCC,0x3F7CBFC9,0x3F7BACCD,
            0x3F7A7302,0x3F791298,0x3F778BC5,0x3F75DEC6,0x3F740BDD,0x3F721352,0x3F6FF573,0x3F6DB293,
            0x3F6B4B0C,0x3F68BF3C,0x3F660F88,0x3F633C5A,0x3F604621,0x3F5D2D53,0x3F59F26A,0x3F5695E5,
            0x3F531849,0x3F4F7A1F,0x3F4BBBF8,0x3F47DE65,0x3F43E200,0x3F3FC767,0x3F3B8F3B,0x3F373A23,
        },{
            0x3F7FFEC4,0x3F7FF4E6,0x3F7FE129,0x3F7FC38F,0x3F7F9C18,0x3F7F6AC7,0x3F7F2F9D,0x3F7EEA9D,
            0x3F7E9BC9,0x3F7E4323,0x3F7DE0B1,0x3F7D7474,0x3F7CFE73,0x3F7C7EB0,0x3F7BF531,0x3F7B61FC,
            0x3F7AC516,0x3F7A1E84,0x3F796E4E,0x3F78B47B,0x3F77F110,0x3F772417,0x3F764D97,0x3F756D97,
            0x3F748422,0x3F73913F,0x3F7294F8,0x3F718F57,0x3F708066,0x3F6F6830,0x3F6E46BE,0x3F6D1C1D,
            0x3F6BE858,0x3F6AAB7B,0x3F696591,0x3F6816A8,0x3F66BECC,0x3F655E0B,0x3F63F473,0x3F628210,
            0x3F6106F2,0x3F5F8327,0x3F5DF6BE,0x3F5C61C7,0x3F5AC450,0x3F591E6A,0x3F577026,0x3F55B993,
            0x3F53FAC3,0x3F5233C6,0x3F5064AF,0x3F4E8D90,0x3F4CAE79,0x3F4AC77F,0x3F48D8B3,0x3F46E22A,
            0x3F44E3F5,0x3F42DE29,0x3F40D0DA,0x3F3EBC1B,0x3F3CA003,0x3F3A7CA4,0x3F385216,0x3F36206C,
        }
    };
    static unsigned int list2Int[7][0x40] = {
        {
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
        },{
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
        },{
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
        },{
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
        },{
            0xBCC90AB0,0xBD96A905,0xBDFAB273,0xBE2F10A2,0xBE605C13,0xBE888E93,0xBEA09AE5,0xBEB8442A,
            0xBECF7BCA,0xBEE63375,0xBEFC5D27,0xBF08F59B,0xBF13682A,0xBF1D7FD1,0xBF273656,0xBF3085BB,
            0x3CC90AB0,0x3D96A905,0x3DFAB273,0x3E2F10A2,0x3E605C13,0x3E888E93,0x3EA09AE5,0x3EB8442A,
            0x3ECF7BCA,0x3EE63375,0x3EFC5D27,0x3F08F59B,0x3F13682A,0x3F1D7FD1,0x3F273656,0x3F3085BB,
            0x3CC90AB0,0x3D96A905,0x3DFAB273,0x3E2F10A2,0x3E605C13,0x3E888E93,0x3EA09AE5,0x3EB8442A,
            0x3ECF7BCA,0x3EE63375,0x3EFC5D27,0x3F08F59B,0x3F13682A,0x3F1D7FD1,0x3F273656,0x3F3085BB,
            0xBCC90AB0,0xBD96A905,0xBDFAB273,0xBE2F10A2,0xBE605C13,0xBE888E93,0xBEA09AE5,0xBEB8442A,
            0xBECF7BCA,0xBEE63375,0xBEFC5D27,0xBF08F59B,0xBF13682A,0xBF1D7FD1,0xBF273656,0xBF3085BB,
        },{
            0xBC490E90,0xBD16C32C,0xBD7B2B74,0xBDAFB680,0xBDE1BC2E,0xBE09CF86,0xBE22ABB6,0xBE3B6ECF,
            0xBE541501,0xBE6C9A7F,0xBE827DC0,0xBE8E9A22,0xBE9AA086,0xBEA68F12,0xBEB263EF,0xBEBE1D4A,
            0xBEC9B953,0xBED53641,0xBEE0924F,0xBEEBCBBB,0xBEF6E0CB,0xBF00E7E4,0xBF064B82,0xBF0B9A6B,
            0xBF10D3CD,0xBF15F6D9,0xBF1B02C6,0xBF1FF6CB,0xBF24D225,0xBF299415,0xBF2E3BDE,0xBF32C8C9,
            0x3C490E90,0x3D16C32C,0x3D7B2B74,0x3DAFB680,0x3DE1BC2E,0x3E09CF86,0x3E22ABB6,0x3E3B6ECF,
            0x3E541501,0x3E6C9A7F,0x3E827DC0,0x3E8E9A22,0x3E9AA086,0x3EA68F12,0x3EB263EF,0x3EBE1D4A,
            0x3EC9B953,0x3ED53641,0x3EE0924F,0x3EEBCBBB,0x3EF6E0CB,0x3F00E7E4,0x3F064B82,0x3F0B9A6B,
            0x3F10D3CD,0x3F15F6D9,0x3F1B02C6,0x3F1FF6CB,0x3F24D225,0x3F299415,0x3F2E3BDE,0x3F32C8C9,
        },{
            0xBBC90F88,0xBC96C9B6,0xBCFB49BA,0xBD2FE007,0xBD621469,0xBD8A200A,0xBDA3308C,0xBDBC3AC3,
            0xBDD53DB9,0xBDEE3876,0xBE039502,0xBE1008B7,0xBE1C76DE,0xBE28DEFC,0xBE354098,0xBE419B37,
            0xBE4DEE60,0xBE5A3997,0xBE667C66,0xBE72B651,0xBE7EE6E1,0xBE8586CE,0xBE8B9507,0xBE919DDD,
            0xBE97A117,0xBE9D9E78,0xBEA395C5,0xBEA986C4,0xBEAF713A,0xBEB554EC,0xBEBB31A0,0xBEC1071E,
            0xBEC6D529,0xBECC9B8B,0xBED25A09,0xBED8106B,0xBEDDBE79,0xBEE363FA,0xBEE900B7,0xBEEE9479,
            0xBEF41F07,0xBEF9A02D,0xBEFF17B2,0xBF0242B1,0xBF04F484,0xBF07A136,0xBF0A48AD,0xBF0CEAD0,
            0xBF0F8784,0xBF121EB0,0xBF14B039,0xBF173C07,0xBF19C200,0xBF1C420C,0xBF1EBC12,0xBF212FF9,
            0xBF239DA9,0xBF26050A,0xBF286605,0xBF2AC082,0xBF2D1469,0xBF2F61A5,0xBF31A81D,0xBF33E7BC,
        }
    };
    static unsigned int list3Int[2][0x40] = {
        {
            0x3A3504F0,0x3B0183B8,0x3B70C538,0x3BBB9268,0x3C04A809,0x3C308200,0x3C61284C,0x3C8B3F17,
            0x3CA83992,0x3CC77FBD,0x3CE91110,0x3D0677CD,0x3D198FC4,0x3D2DD35C,0x3D434643,0x3D59ECC1,
            0x3D71CBA8,0x3D85741E,0x3D92A413,0x3DA078B4,0x3DAEF522,0x3DBE1C9E,0x3DCDF27B,0x3DDE7A1D,
            0x3DEFB6ED,0x3E00D62B,0x3E0A2EDA,0x3E13E72A,0x3E1E00B1,0x3E287CF2,0x3E335D55,0x3E3EA321,
            0x3E4A4F75,0x3E56633F,0x3E62DF37,0x3E6FC3D1,0x3E7D1138,0x3E8563A2,0x3E8C72B7,0x3E93B561,
            0x3E9B2AEF,0x3EA2D26F,0x3EAAAAAB,0x3EB2B222,0x3EBAE706,0x3EC34737,0x3ECBD03D,0x3ED47F46,
            0x3EDD5128,0x3EE6425C,0x3EEF4EFF,0x3EF872D7,0x3F00D4A9,0x3F0576CA,0x3F0A1D3B,0x3F0EC548,
            0x3F136C25,0x3F180EF2,0x3F1CAAC2,0x3F213CA2,0x3F25C1A5,0x3F2A36E7,0x3F2E9998,0x3F32E705,
        },{
            0xBF371C9E,0xBF3B37FE,0xBF3F36F2,0xBF431780,0xBF46D7E6,0xBF4A76A4,0xBF4DF27C,0xBF514A6F,
            0xBF547DC5,0xBF578C03,0xBF5A74EE,0xBF5D3887,0xBF5FD707,0xBF6250DA,0xBF64A699,0xBF66D908,
            0xBF68E90E,0xBF6AD7B1,0xBF6CA611,0xBF6E5562,0xBF6FE6E7,0xBF715BEF,0xBF72B5D1,0xBF73F5E6,
            0xBF751D89,0xBF762E13,0xBF7728D7,0xBF780F20,0xBF78E234,0xBF79A34C,0xBF7A5397,0xBF7AF439,
            0xBF7B8648,0xBF7C0ACE,0xBF7C82C8,0xBF7CEF26,0xBF7D50CB,0xBF7DA88E,0xBF7DF737,0xBF7E3D86,
            0xBF7E7C2A,0xBF7EB3CC,0xBF7EE507,0xBF7F106C,0xBF7F3683,0xBF7F57CA,0xBF7F74B6,0xBF7F8DB6,
            0xBF7FA32E,0xBF7FB57B,0xBF7FC4F6,0xBF7FD1ED,0xBF7FDCAD,0xBF7FE579,0xBF7FEC90,0xBF7FF22E,
            0xBF7FF688,0xBF7FF9D0,0xBF7FFC32,0xBF7FFDDA,0xBF7FFEED,0xBF7FFF8F,0xBF7FFFDF,0xBF7FFFFC,
        }
    };
    float *s, *d, *s1, *s2;
    s = block; d = wav1;
    for (int i = 0, count1 = 1, count2 = 0x40; i<7; i++, count1 <<= 1, count2 >>= 1) {
        float *d1 = d;
        float *d2 = &d[count2];
        for (int j = 0; j<count1; j++) {
            for (int k = 0; k<count2; k++) {
                float a = *(s++);
                float b = *(s++);
                *(d1++) = b + a;
                *(d2++) = a - b;
            }
            d1 += count2;
            d2 += count2;
        }
        float *w = &s[-0x80]; s = d; d = w;
    }
    s = wav1; d = block;
    for (int i = 0, count1 = 0x40, count2 = 1; i<7; i++, count1 >>= 1, count2 <<= 1) {
        float *list1Float = (float *)list1Int[i];
        float *list2Float = (float *)list2Int[i];
        float *s1 = s;
        float *s2 = &s1[count2];
        float *d1 = d;
        float *d2 = &d1[count2 * 2 - 1];
        for (int j = 0; j<count1; j++) {
            for (int k = 0; k<count2; k++) {
                float a = *(s1++);
                float b = *(s2++);
                float c = *(list1Float++);
                float d = *(list2Float++);
                *(d1++) = a * c - b * d;
                *(d2--) = a * d + b * c;
            }
            s1 += count2;
            s2 += count2;
            d1 += count2;
            d2 += count2 * 3;
        }
        float *w = s; s = d; d = w;
    }
    memcpy(wav2, s, 0x80 * sizeof(float));
    //for (int i = 0; i<0x80; i++)*(d++) = *(s++);
    s = (float *)list3Int; d = wave[index];
    s1 = &wav2[0x40]; s2 = wav3;
    for (int i = 0; i<0x40; i++)*(d++) = *(s1++)**(s++) + *(s2++);
    for (int i = 0; i<0x40; i++)*(d++) = *(s++)**(--s1) - *(s2++);
    s1 = &wav2[0x40 - 1]; s2 = wav3;
    for (int i = 0; i<0x40; i++)*(s2++) = *(s1--)**(--s);
    for (int i = 0; i<0x40; i++)*(s2++) = *(--s)**(++s1);
}
