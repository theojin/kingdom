#include <stdio.h>
#include <stdbool.h>

#include "nlp.h"
#include "log.h"

struct nlp_information {
	char* szSttText;	// STT에서 발화->Text로 변환된 문장
	char* szNounText;	// 추출된 명사 Text
	char* szVerbText;	// 추출된 동사 Text
	char* szTimeText;	// 추출된 시간 Text
	char* szTtsText;	// TTS로 전달될 Text

	int nNoun_No;		// 명사 고유 번호(1~4)
	int nVerb_No;		// 동사 고유 번호(1~14)
	int nTimeInfo;		// 알람 시간 정보(1~60)
	int nErrorCode;	// NLP 처리 결과 (0:실패, 1:성공)
};

struct nlp_information *nlp(char *string)
{
	return NULL;
}
