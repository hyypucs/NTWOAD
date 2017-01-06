/*

        Comment:
*/
#if !defined(__HTML_H__)
#define __HTML_H__

#define MAX_TAG_LEN		1024
PUBLIC tBOOL getHtmlTitle(tHCHAR *uszHtmlStr, tHCHAR *uszTitle);
PUBLIC tVOID deleteTag(tHCHAR *uszBuf);
PUBLIC tVOID DeleteTag(tHCHAR *uszBuf);
PUBLIC tVOID convSpecialChr(tHCHAR *uszHtmlStr);
PUBLIC tVOID deleteSpecialTag(tHCHAR *uszHtmlStr);
PUBLIC tVOID deleteHtmlTitle(tHCHAR *uszHtmlStr);
PUBLIC tVOID DeleteTagExceptBR(tHCHAR *uszBuf, tINT nOpt);

#endif /* html.h */
