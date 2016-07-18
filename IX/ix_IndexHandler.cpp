//
// Created by 谢俊东 on 16/7/17.
//

#include <cassert>
#include "ix.h"

RC IX_IndexHandler::InsertEntry(void *pData, const RID &rid) {
    if(ix_fh.rootNode == -1) {
        //如果root node不存在,新建一个root node
        IX_PageHandler rootPH;
        AllocatePage(rootPH);
        rootPH.InitPage(NO_PARENT_NODE, -1, LeafPage);
        ix_fh.rootNode = rootPH.GetPageNum();
    }
    Key key((char *) pData, ix_fh.attrType, ix_fh.attrLength);
    int leaf = Search(ix_fh.rootNode, key);     //找到entry应该在哪个page插入
    Insert_into_Leaf(leaf, key, rid);
    MarkDirty(leaf);
//    WriteToBlock();
    return 0;
}

int IX_IndexHandler::Insert_into_Leaf(int page, const Key &key, const RID &rid) {
    IX_PageHandler ix_PH;
    if (GetThisPage(page, ix_PH) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    if (ix_PH.ix_pageHeader.nCurPtrs < ix_fh.nMaxPtrLeafPage) {
        //如果当前叶节点未满
        ix_PH.InsertLeafEntry(key, rid);

    } else if (ix_PH.ix_pageHeader.nCurPtrs >= ix_fh.nMaxPtrLeafPage) {

        IX_PageHandler ix_newLeafPH;
        AllocatePage(ix_newLeafPH);    //分配一个新的Page
        ix_newLeafPH.InitPage(ix_PH.GetParentNode(), ix_PH.ix_pageHeader.nextNode,
                              LeafPage);
        //初始化新Page
        ix_PH.ix_pageHeader.nextNode = ix_newLeafPH.GetPageNum();

        ix_PH.InsertLeafEntry(key, rid);
        //后一半复制到新的Page中
        int nEntries = ix_PH.ix_pageHeader.nCurPtrs;
        Key mid_key = ix_PH.GetLeafKey(nEntries / 2);
        memcpy(ix_newLeafPH.GetDataPtr(), ix_PH.GetDataPtr() + (nEntries / 2) * (ix_fh.attrLength + sizeof(RID)),
               (nEntries  - (nEntries / 2)) * (ix_fh.attrLength + sizeof(RID)));
        //更新两个node的当前指针数
        ix_PH.ix_pageHeader.nCurPtrs /= 2;
        ix_newLeafPH.ix_pageHeader.nCurPtrs = nEntries - ix_PH.ix_pageHeader.nCurPtrs;

//        for (int i = nEntries / 2; i < nEntries; i++)
//            ix_newLeafPH.leafNode.entries.push_back(ix_PH.leafNode.entries[i]);
//        ix_PH.leafNode.entries.resize(nEntries / 2);   //缩小到一半

        IX_PageHandler ix_parentPH;
        if (ix_PH.ix_pageHeader.parentNode != NO_PARENT_NODE) {
            //如果存在parent node
            GetThisPage(ix_PH.ix_pageHeader.parentNode, ix_parentPH);
        } else {
            //如果不存在parent node,新建一个parentNode

            AllocatePage(ix_parentPH);
            ix_parentPH.InitPage(NO_PARENT_NODE, -1, InteriorPage);
//            ix_parentPH.interNode.pointers.push_back(ix_PH.GetPageNum());
            *(PagePtr *)ix_parentPH.GetDataPtr() = ix_PH.GetPageNum();  //单个指针指向当前leaf node
            ix_fh.rootNode = ix_parentPH.GetPageNum();
        }
        Insert_into_interior(ix_parentPH.GetPageNum(), mid_key, ix_newLeafPH.GetPageNum());
        MarkDirty(ix_parentPH.GetPageNum());
        UnpinPage(ix_parentPH.GetPageNum());
    }
    return 0;
}

int IX_IndexHandler::Insert_into_interior(int target_page, const Key &key, PagePtr pPage) {
    IX_PageHandler ix_pageHdlr;
    if (GetThisPage(target_page, ix_pageHdlr) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    if (ix_pageHdlr.ix_pageHeader.nCurPtrs < ix_fh.nMaxPtrInteriorPage) {
        //如果当前节点未满
        ix_pageHdlr.InsertInteriorEntry(key, pPage);

    } else if (ix_pageHdlr.ix_pageHeader.nCurPtrs >= ix_fh.nMaxPtrInteriorPage) {
        IX_PageHandler ix_newPageHdlr;
        AllocatePage(ix_newPageHdlr);    //分配一个新的Page
        ix_newPageHdlr.InitPage(ix_pageHdlr.ix_pageHeader.parentNode, ix_pageHdlr.ix_pageHeader.nextNode,
                                LeafPage);
        //初始化新Page
        ix_pageHdlr.InsertInteriorEntry(key, pPage);

        int total = ix_pageHdlr.ix_pageHeader.nCurPtrs;
        int mid = (ix_pageHdlr.ix_pageHeader.nCurPtrs - 1) / 2;
        Key mid_key = ix_pageHdlr.GetInteriorKey(mid);  //把中间的key拎出来
        int interior_entry_size = ix_fh.attrLength + sizeof(PagePtr);
        //复制一半到新page中
        memcpy(ix_newPageHdlr.GetDataPtr(), ix_pageHdlr.GetDataPtr() + (mid + 1) * (interior_entry_size),
               (ix_pageHdlr.ix_pageHeader.nCurPtrs - 1 - mid - 1) * interior_entry_size + sizeof(PagePtr));
        //更新两个Node的当前指针数
        ix_pageHdlr.ix_pageHeader.nCurPtrs = mid;
        ix_newPageHdlr.ix_pageHeader.nCurPtrs = total - mid;
//        int mid = ix_pageHdlr.interNode.keys.size() / 2;
//        for (i = mid + 1; i < ix_pageHdlr.interNode.keys.size(); i++) {
//            ix_newPageHdlr.interNode.keys.push_back(ix_pageHdlr.interNode.keys[i]);
//        }
//        ix_pageHdlr.interNode.keys.resize(mid);
//        for (i = mid + 1; i < ix_pageHdlr.interNode.pointers.size(); i++) {
//            ix_newPageHdlr.interNode.pointers.push_back(ix_pageHdlr.interNode.pointers[i]);
//        }
//        ix_pageHdlr.interNode.pointers.resize(mid + 1);
        MarkDirty(ix_newPageHdlr.GetPageNum());
        UnpinPage(ix_newPageHdlr.GetPageNum());
        IX_PageHandler parentPH;
        if (ix_pageHdlr.GetParentNode() == NO_PARENT_NODE) {
            AllocatePage(parentPH);
            parentPH.InitPage(NO_PARENT_NODE, -1, InteriorPage);
            *(PagePtr *)parentPH.GetDataPtr() = ix_pageHdlr.GetPageNum();
            ix_fh.rootNode = parentPH.GetPageNum();
        } else {
            GetThisPage(ix_pageHdlr.GetParentNode(), parentPH);
        }

        Insert_into_interior(ix_pageHdlr.GetParentNode(), mid_key, ix_newPageHdlr.GetPageNum());
        MarkDirty(ix_pageHdlr.GetParentNode());
        UnpinPage(ix_pageHdlr.GetParentNode());
    }
    return 0;
}

RC IX_IndexHandler::GetThisPage(int pageNum, IX_PageHandler &ix_pageHandler) {

    PageHandler pf_pageHandler;
    if (pf_fileHandler.GetThisPage(pageNum, pf_pageHandler) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    ix_pageHandler = IX_PageHandler(pf_pageHandler, ix_fh.attrType, ix_fh.attrLength);
    return 0;
}

int IX_IndexHandler::Search(int page, Key key) {
    IX_PageHandler ix_pageHandler;
    if (GetThisPage(page, ix_pageHandler) == PAGE_NOT_IN_USE) return IX_PAGE_NOT_IN_USE;
    int pageType = ix_pageHandler.ix_pageHeader.pageType;
    if (pageType == LeafPage) return page;
    else {
        assert(ix_pageHandler.ix_pageHeader.nCurPtrs >= 2);
        if (key < ix_pageHandler.GetInteriorKey(0))  //如果小于第一个值
            return Search(ix_pageHandler.GetInteriorPtr(0), key);
        else if (key >= ix_pageHandler.GetInteriorKey(ix_pageHandler.ix_pageHeader.nCurPtrs - 2))   //如果大于最后一个值
            return Search(ix_pageHandler.GetInteriorPtr(ix_pageHandler.ix_pageHeader.nCurPtrs - 1), key);

        for (int i = 0; i < ix_pageHandler.ix_pageHeader.nCurPtrs - 1; i++) {
            if (key >= ix_pageHandler.GetInteriorKey(i))
                return Search(ix_pageHandler.GetInteriorPtr(i + 1), key);
        }
    }

}

RC IX_IndexHandler::AllocatePage(IX_PageHandler &ix_pageHandler) {
    PageHandler pf_pageHandler;
    pf_fileHandler.AllocatePage(pf_pageHandler);
    ix_pageHandler = IX_PageHandler(pf_pageHandler, ix_fh.attrType, ix_fh.attrLength);
    ix_pageHandler.ix_pageHeader.nCurPtrs = 0;
}

