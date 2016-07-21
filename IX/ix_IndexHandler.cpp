//
// Created by 谢俊东 on 16/7/17.
//

#include <cassert>
#include "ix.h"

RC IX_IndexHandler::InsertEntry(void *pData, const RID &rid) {
    if(ix_fh.rootNode == -1) {
        //如果root node不存在,新建一个root node,同时也是leaf node
        IX_PageHandler rootPH;
        AllocatePage(rootPH);
        rootPH.InitPage(NO_PARENT_NODE, IX_NEXT_LIST_END, IX_PREV_LIST_END, LeafPage);
        setRootNode(rootPH.GetPageNum());
        UnpinPage(rootPH.GetPageNum());
    }
    Key key((char *) pData, ix_fh.attrType, ix_fh.attrLength);
    int leaf = Search(ix_fh.rootNode, key);     //找到entry应该在哪个page插入
    Insert_into_Leaf(leaf, key, rid);
    MarkDirty(leaf);
    return 0;
}

int IX_IndexHandler::Insert_into_Leaf(int target_page, const Key &key, const RID &rid) {
    IX_PageHandler ix_PH;
    if (GetThisPage(target_page, ix_PH) == PAGE_NOT_IN_USE) {
        printf("page not in use\n");
        return PAGE_NOT_IN_USE;
    }
    if (ix_PH.GetnCurPtr() < ix_fh.nMaxPtrLeafPage) {
        //如果当前叶节点未满
        ix_PH.InsertLeafEntry(key, rid);

    } else if (ix_PH.GetnCurPtr() >= ix_fh.nMaxPtrLeafPage) {

        IX_PageHandler ix_newLeafPH;
        AllocatePage(ix_newLeafPH);    //分配一个新的neighbour Page
        ix_newLeafPH.InitPage(ix_PH.GetParentNode(), ix_PH.GetNextNode(), ix_PH.GetPageNum(),
                              LeafPage);
        //初始化新Page
        ix_PH.setNextNode(ix_newLeafPH.GetPageNum());

        ix_PH.InsertLeafEntry(key, rid);
        //后一半复制到新的Page中
        int nEntries = ix_PH.GetnCurPtr();
        Key mid_key = ix_PH.GetLeafKey(nEntries / 2);
        memcpy(ix_newLeafPH.GetDataPtr(), ix_PH.GetDataPtr() + (nEntries / 2) * (ix_fh.attrLength + sizeof(RID)),
               (nEntries  - (nEntries / 2)) * (ix_fh.attrLength + sizeof(RID)));
        //更新两个node的当前指针数
        ix_PH.setCurPtr(ix_PH.GetnCurPtr() / 2);
        ix_newLeafPH.setCurPtr(nEntries - ix_PH.GetnCurPtr());


        IX_PageHandler ix_parentPH;
        if (ix_PH.GetParentNode() != NO_PARENT_NODE) {
            //如果存在parent node, 直接获取
            GetThisPage(ix_PH.GetParentNode(), ix_parentPH);
            ix_newLeafPH.setParentNode(ix_parentPH.GetPageNum());
            UnpinPage(ix_parentPH.GetPageNum());
        } else {
            //如果不存在parent node,新建一个parentNode

            AllocatePage(ix_parentPH);
            ix_parentPH.InitPage(NO_PARENT_NODE, NO_NEXT_NODE, NO_PREV_NODE, InteriorPage);
            *(PagePtr *)ix_parentPH.GetDataPtr() = ix_PH.GetPageNum();  //单个指针指向当前leaf node
            ix_parentPH.increaseCurPtr();
            setRootNode(ix_parentPH.GetPageNum());  //更新rootNode
            ix_PH.setParentNode(ix_parentPH.GetPageNum());
            ix_newLeafPH.setParentNode(ix_parentPH.GetPageNum());
            MarkDirty(ix_parentPH.GetPageNum());
            UnpinPage(ix_parentPH.GetPageNum());
        }
        MarkDirty(ix_newLeafPH.GetPageNum());
        UnpinPage(ix_newLeafPH.GetPageNum());

        Insert_into_Interior(ix_parentPH.GetPageNum(), mid_key, ix_newLeafPH.GetPageNum());
    }
    MarkDirty(ix_PH.GetPageNum());
    UnpinPage(ix_PH.GetPageNum());
    return 0;
}

int IX_IndexHandler::Insert_into_Interior(int target_page, const Key &key, PagePtr pPage) {
    IX_PageHandler ix_pageHdlr;
    if (GetThisPage(target_page, ix_pageHdlr) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    if (ix_pageHdlr.GetnCurPtr() < ix_fh.nMaxPtrInteriorPage) {
        //如果当前节点未满
        ix_pageHdlr.InsertInteriorEntry(key, pPage);

    } else if (ix_pageHdlr.GetnCurPtr() >= ix_fh.nMaxPtrInteriorPage) {
        IX_PageHandler ix_newPageHdlr;
        AllocatePage(ix_newPageHdlr);    //分配一个新的Page
        ix_newPageHdlr.InitPage(ix_pageHdlr.GetParentNode(), NO_NEXT_NODE, NO_PREV_NODE,
                                InteriorPage);
        //初始化新Page
        ix_pageHdlr.InsertInteriorEntry(key, pPage);

        int total = ix_pageHdlr.GetnCurPtr();
        int mid = (total - 1) / 2;
        Key mid_key = ix_pageHdlr.GetInteriorKey(mid);  //把中间的key拎出来
        int interior_entry_size = ix_fh.attrLength + sizeof(PagePtr);
        //复制一半到新page中
        memcpy(ix_newPageHdlr.GetDataPtr(), ix_pageHdlr.GetDataPtr() + (mid + 1) * (interior_entry_size),
               (total - 1 - (mid + 1)) * interior_entry_size + sizeof(PagePtr));
        //更新两个Node的当前指针数
        ix_pageHdlr.setCurPtr(mid + 1);
        ix_newPageHdlr.setCurPtr(total - (mid + 1));
        UpdateParentPtrOfChildNode(ix_newPageHdlr);

        IX_PageHandler parentPH;
        if (ix_pageHdlr.GetParentNode() == NO_PARENT_NODE) {
            AllocatePage(parentPH);
            parentPH.InitPage(NO_PARENT_NODE, NO_NEXT_NODE, NO_PREV_NODE, InteriorPage);
            *(PagePtr *)parentPH.GetDataPtr() = ix_pageHdlr.GetPageNum();
            parentPH.increaseCurPtr();

            setRootNode(parentPH.GetPageNum());
            ix_pageHdlr.setParentNode(parentPH.GetPageNum());
            ix_newPageHdlr.setParentNode(parentPH.GetPageNum());
            MarkDirty(parentPH.GetPageNum());
            UnpinPage(parentPH.GetPageNum());
        } else {
            GetThisPage(ix_pageHdlr.GetParentNode(), parentPH);
            ix_newPageHdlr.setParentNode(parentPH.GetPageNum());
            UnpinPage(parentPH.GetPageNum());
        }

        MarkDirty(ix_newPageHdlr.GetPageNum());
        UnpinPage(ix_newPageHdlr.GetPageNum());
        if(parentPH.GetPageNum() == 41) {

        }
        Insert_into_Interior(parentPH.GetPageNum(), mid_key, ix_newPageHdlr.GetPageNum());
    }
    MarkDirty(ix_pageHdlr.GetPageNum());
    UnpinPage(ix_pageHdlr.GetPageNum());
    return 0;
}

RC IX_IndexHandler::GetThisPage(int pageNum, IX_PageHandler &ix_pageHandler) {

    PageHandler pf_pageHandler;
    if (pf_fileHandler.GetThisPage(pageNum, pf_pageHandler) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    ix_pageHandler = IX_PageHandler(pf_pageHandler, ix_fh.attrType, ix_fh.attrLength);
    return 0;
}

int IX_IndexHandler::Search(int page, const Key &key) {
    IX_PageHandler ix_pageHandler;
    if (GetThisPage(page, ix_pageHandler) == PAGE_NOT_IN_USE) {
        printf("IX_PAGE_NOT_IN_USE\n");
        exit(234);
//        return IX_PAGE_NOT_IN_USE;
    }
    int pageType = ix_pageHandler.GetPageType();
    int result_page;
    if (pageType == LeafPage) {
        UnpinPage(page);
        return page;
    }
    else {
        if(ix_pageHandler.GetnCurPtr() == 1) {
            result_page = Search(ix_pageHandler.GetInteriorPtr(0), key);
        }
        else if (key < ix_pageHandler.GetInteriorKey(0))  //如果小于第一个值
            result_page = Search(ix_pageHandler.GetInteriorPtr(0), key);
        else if (key >= ix_pageHandler.GetInteriorKey(ix_pageHandler.GetnCurPtr() - 2))   //如果大于最后一个值
            result_page = Search(ix_pageHandler.GetInteriorPtr(ix_pageHandler.GetnCurPtr() - 1), key);

        else {
            for (int i = 0; i < ix_pageHandler.GetnCurPtr() - 2; i++) {
                if (key >= ix_pageHandler.GetInteriorKey(i) && key < ix_pageHandler.GetInteriorKey(i + 1)) {
                    result_page = Search(ix_pageHandler.GetInteriorPtr(i + 1), key);
                    break;
                }
            }
        }
        UnpinPage(ix_pageHandler.GetPageNum());
    }
    return result_page;

}

RC IX_IndexHandler::AllocatePage(IX_PageHandler &ix_pageHandler) {
    PageHandler pf_pageHandler;
    if(pf_fileHandler.AllocatePage(pf_pageHandler) == BM_NO_FREE_BUF_WARNING)
        return BM_NO_FREE_BUF_WARNING;
    ix_pageHandler = IX_PageHandler(pf_pageHandler, ix_fh.attrType, ix_fh.attrLength);
    ix_pageHandler.setCurPtr(0);
    return 0;

}


RC IX_IndexHandler::DeleteEntry(void *pData, const RID &rid) {
    Key key((char *)pData, ix_fh.attrType, ix_fh.attrLength);
    int page = Search(ix_fh.rootNode, key);
    IX_PageHandler ix_PH;
    if (GetThisPage(page, ix_PH) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    ix_PH.DeleteLeafEntry(key);
    if(ix_PH.GetnCurPtr() == 0) {
        //如果当前页面为空页面
        int parent = ix_PH.GetParentNode();
        if(parent != NO_PARENT_NODE) {
            Delete_from_Interior(parent, ix_PH.GetPageNum());
        }
        IX_PageHandler ix_neighbourPH;
        assert(ix_PH.GetPageType() == LeafPage);
        if(ix_PH.GetNextNode() != IX_NEXT_LIST_END) {
            if (GetThisPage(ix_PH.GetNextNode(), ix_neighbourPH) == PAGE_NOT_IN_USE)
                return PAGE_NOT_IN_USE;
            ix_neighbourPH.setPrevNode(ix_PH.GetPrevNode());
            MarkDirty(ix_neighbourPH.GetPageNum());
            UnpinPage(ix_neighbourPH.GetPageNum());
        }
        if(ix_PH.GetPrevNode() != IX_PREV_LIST_END) {
            if(GetThisPage(ix_PH.GetPrevNode(), ix_neighbourPH) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
            ix_neighbourPH.setNextNode(ix_PH.GetNextNode());
            MarkDirty(ix_neighbourPH.GetPageNum());
            UnpinPage(ix_neighbourPH.GetPageNum());
        }

        MarkDirty(ix_neighbourPH.GetPageNum());
        UnpinPage(ix_neighbourPH.GetPageNum());
        DisposePage(ix_PH.GetPageNum());
    }
    UnpinPage(page);
    return 0;
}

RC IX_IndexHandler::Delete_from_Interior(int target_page, PagePtr child_page) {
    IX_PageHandler ix_PH;
    int i;
    if (GetThisPage(target_page, ix_PH) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    if(ix_PH.GetnCurPtr() == 1) {   //如果只剩下一个指针
        if(ix_PH.GetInteriorPtr(0) == child_page) {
            ix_PH.setCurPtr(0);
            if(ix_PH.GetParentNode() != NO_PARENT_NODE) {   //如果存在父节点
                Delete_from_Interior(ix_PH.GetParentNode(), ix_PH.GetPageNum());
                DisposePage(ix_PH.GetPageNum());
            } else {    //如果没有父节点,当前节点是根节点
                DisposePage(ix_PH.GetPageNum());
                setRootNode(NO_ROOT_NODE);
            }
            return 0;
        }
        UnpinPage(ix_PH.GetPageNum());
        return IX_NO_SUCH_ENTRY;
    }
    assert(ix_PH.GetnCurPtr() > 1);
    for(i = 0; i < ix_PH.GetnCurPtr(); i++) {
        if( ix_PH.GetInteriorPtr(i) == child_page) {
            int interior_entry_size = ix_fh.attrLength + sizeof(PagePtr);
            if(i < ix_PH.GetnCurPtr() - 1) {    //如果不是最后一个指针
                memmove(ix_PH.GetDataPtr() + i * interior_entry_size,
                        ix_PH.GetDataPtr() + (i + 1) * interior_entry_size,
                        (ix_PH.ix_pageHeader.nCurPtrs - 2 - i) * (ix_fh.attrLength + sizeof(PagePtr)) + sizeof(PagePtr));
            }
            ix_PH.setCurPtr(ix_PH.GetnCurPtr() - 1);
            break;
        }
    }
    MarkDirty(ix_PH.GetPageNum());
    UnpinPage(ix_PH.GetPageNum());
    if(i == ix_PH.GetnCurPtr()) return IX_NO_SUCH_ENTRY;
    else return 0;
}

void IX_IndexHandler::PrintAll() {
    int root = ix_fh.rootNode;
    if(root == NO_ROOT_NODE) {
        printf("there is no node\n");
    } else {
        PrintInterNode(root);

    }
}

void IX_IndexHandler::PrintInterNode(int node) {
    IX_PageHandler ix_interiorPage;
    IX_PageHandler ix_pageHandler;
    GetThisPage(node, ix_interiorPage);
    if(ix_interiorPage.GetPageType() == LeafPage) {
        ix_interiorPage.PrintLeafEntries();
        UnpinPage(ix_interiorPage.GetPageNum());
        return;
    }
    else {
        ix_interiorPage.PrintInteriorEntries();
    }
    int page;
    for(int i = 0; i < ix_interiorPage.GetnCurPtr(); i++) {
        page = ix_interiorPage.GetInteriorPtr(i);
        GetThisPage(page, ix_pageHandler);
        if(ix_pageHandler.GetPageType() == LeafPage) {
            ix_pageHandler.PrintLeafEntries();
        }
        else {
            PrintInterNode(page);
        }
        UnpinPage(ix_pageHandler.GetPageNum());
    }

    UnpinPage(ix_interiorPage.GetPageNum());
}

RC IX_IndexHandler::UpdateParentPtrOfChildNode(IX_PageHandler &ix_pageHandler) {

    IX_PageHandler child_PH;
    for(int i = 0; i < ix_pageHandler.GetnCurPtr(); i++) {
        if(GetThisPage(ix_pageHandler.GetInteriorPtr(i), child_PH) == PAGE_NOT_IN_USE) {
            printf("UpdateParentPtrOfChildNode GetThisPage failure\n");
            return PAGE_NOT_IN_USE;
        }
        child_PH.setParentNode(ix_pageHandler.GetPageNum());
        UnpinPage(child_PH.GetPageNum());
    }
}