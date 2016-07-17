//
// Created by 谢俊东 on 16/7/17.
//

#include "ix.h"

RC IX_IndexHandler::InsertEntry(void *pData, const RID &rid) {
    Key key((char *)pData, ix_fh.attrType, ix_fh.attrLength);
    int leaf = Search(ix_fh.rootNode, key);     //找到entry应该在哪个page插入
    return Insert_into_Leaf(leaf, key, rid);
    return 0;
}

int IX_IndexHandler::Insert_into_Leaf(int page, Key key, const RID &rid) {
    IX_PageHandler ix_pageHandler;
    if( GetThisPage(page, ix_pageHandler) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    if( ix_pageHandler.ix_pageHeader.n_curPtrs < ix_fh.nMaxPtrLeafPage) {
        //如果当前叶节点未满
        ix_pageHandler.InsertLeafEntry(key, rid);
    } else if() {
        //如果当前叶节点满了,父节点未满

    } else if() {
        //如果当前叶节点和父节点都满了
    }
}

RC IX_IndexHandler::GetThisPage(int pageNum, IX_PageHandler &ix_pageHandler){

    PageHandler pf_pageHandler;
    if( pf_fileHandler.GetThisPage(pageNum, pf_pageHandler) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    ix_pageHandler = IX_PageHandler(pf_pageHandler, ix_fh.attrType, ix_fh.attrLength);
    return 0;
}

int IX_IndexHandler::Search(int page, Key key) {
    IX_PageHandler ix_pageHandler;
    if( GetThisPage(page, ix_pageHandler) == PAGE_NOT_IN_USE) return IX_PAGE_NOT_IN_USE;
    int pageType = ix_pageHandler.ix_pageHeader.pageType;
    if(pageType == LeafPage) return page;
    else {
        if( key < ix_pageHandler.internalNode.keys[0])  //如果小于第一个值
            return Search(ix_pageHandler.internalNode.pointers[0], key);
        else if( key >= ix_pageHandler.internalNode.keys[-1])   //如果大于最后一个值
            return Search(ix_pageHandler.internalNode.pointers[-1], key);

        for(int i = 0; i < ix_pageHandler.internalNode.keys.size(); i++) {
            if(ix_pageHandler.internalNode.keys[i] <= key < ix_pageHandler.internalNode.keys[i + 1])
                return Search(ix_pageHandler.internalNode.pointers[i + 1], key);
        }
    }

}

