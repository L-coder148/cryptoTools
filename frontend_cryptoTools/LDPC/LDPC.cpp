
#define NDEBUG
#include "LDPC.h"
#include <cryptoTools/Common/Matrix.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Common/TestCollection.h>
#include <random>
#include <numeric>
#include <sstream>
//#include "sparsehash/dense_hash_set"
//#include "sparsehash/sparse_hash_set"

//#include "../flat_hash_map/bytell_hash_map.hpp"
//#include "../hopscotch-map/include/tsl/bhopscotch_set.h"
//#include "../hopscotch-map/include/tsl/hopscotch_set.h"
//#include "../ordered-map/include/tsl/ordered_set.h"
//#include "../sparse-map/include/tsl/sparse_set.h"
//#include "../robin-map/include/tsl/robin_set.h"
//#include "../cpp-btree/btree/set.h"

#include "cryptoTools/Common/CuckooIndex.h"
#include <unordered_set>
//#include "absl/container/btree_set.h"
//#include "absl/container/flat_hash_set.h"
//#include "absl/container/node_hash_set.h"

#define LDPC_DEBUG

namespace osuCrypto
{


    void print(std::ostream& o, const MatrixView<u64>& rows, u64 cols)
    {

        for (u64 i = 0; i < rows.rows(); ++i)
        {
            std::unordered_set<u64> c;
            for (u64 j = 0; j < rows.cols(); j++)
                c.insert(rows(i, j));

            for (u64 j = 0; j < cols; ++j)
            {
                if (c.find(j) != c.end())
                {
                    o << "1 ";
                }
                else
                {
                    o << ". ";
                }
            }
            o << "\n";
        }

        o << "\n";
    }

    std::ostream& operator<<(std::ostream& o, const LDPC& s)
    {
        print(o, s.mRows, s.cols());
        return o;
    }


    struct ColorBuff
    {
        std::ostream& mO;
        oc::Color mCur = oc::Color::Default;
        std::stringstream mSS;

        ColorBuff(std::ostream& o)
            : mO(o)
        {}

        ~ColorBuff()
        {
            mO << mSS.str();
        }

        ColorBuff& operator<<(const oc::Color& t)
        {
            if (mCur != t)
            {
                mO << mSS.str() << t;
                mSS = std::stringstream{};
                mCur = t;
            }
            return *this;
        }


        template<typename T>
        ColorBuff& operator<<(const T& t)
        {
            mSS << t;
            return *this;
        }
    };

    //std::ostream& operator<<(std::ostream& o, const diff& s)
    //{
    //    std::array<oc::Color, 2> colors{ oc::Color::Blue, oc::Color::Green };
    //    u8 rowColorIdx = 0;
    //    for (u64 i = 0; i < s.mL.rows(); ++i)
    //    {

    //        std::unordered_set<u64> lc, rc;
    //        for (u64 j = 0; j < s.mL.cols(); j++)
    //            lc.insert(s.mL(i, j));
    //        for (u64 j = 0; j < s.mR.cols(); j++)
    //            rc.insert(s.mR(i, j));

    //        auto diffCols = lc;
    //        for (auto c : rc)
    //        {
    //            auto iter = diffCols.find(c);
    //            if (iter == diffCols.end())
    //                diffCols.insert(c);
    //            else
    //                diffCols.erase(iter);
    //        }

    //        //if (std::find(s.mRIdx.begin(), s.mRIdx.end(), i) != s.mRIdx.end())
    //        //{
    //        //    rowColorIdx ^= 1;
    //        //}

    //        auto colorIdx = rowColorIdx;
    //        for (u64 j = 0; j < s.mL.cols(); ++j)
    //        {

    //            //if (std::find(s.mCIdx.begin(), s.mCIdx.end(), j) != s.mCIdx.end())
    //            //{
    //            //    colorIdx ^= 1;
    //            //}


    //            if (diffCols.find(j) != diffCols.end())
    //                o << oc::Color::Red;
    //            else
    //                o << colors[colorIdx];

    //            if (lc.find(j) != lc.end())
    //            {
    //                o << "1 ";
    //            }
    //            else
    //            {
    //                o << "0 ";
    //            }
    //            o << oc::Color::Default;
    //        }

    //        if (s.mWeights)
    //            o << "   " << (*s.mWeights)[i];
    //        o << "\n";
    //    }

    //    return o;
    //}

    std::ostream& operator<<(std::ostream& o, const diff& s)
    {
        std::array<oc::Color, 2> colors{ oc::Color::Blue, oc::Color::Red };

        //ColorBuff o(oo);
        u8 rowColorIdx = 0;
        std::stringstream ss;
        for (u64 i = 0; i < s.mL.rows(); ++i)
        {

            std::unordered_set<u64> lc, rc;
            for (u64 j = 0; j < s.mL.cols(); j++)
                lc.insert(s.mL(i, j));
            for (u64 j = 0; j < s.mR.cols(); j++)
                rc.insert(s.mR(i, j));

            auto diffCols = lc;
            for (auto c : rc)
            {
                auto iter = diffCols.find(c);
                if (iter == diffCols.end())
                    diffCols.insert(c);
                else
                    diffCols.erase(iter);
            }

            for (u64 k = 0; k < s.mBlocks.size(); ++k)
            {
                if (s.mBlocks[k][0] == i)
                {
                    rowColorIdx ^= 1;
                    break;
                }
            }
            auto colorIdx = rowColorIdx;
            for (u64 j = 0; j < s.mNumCols; ++j)
            {
                for (u64 k = 0; k < s.mBlocks.size(); ++k)
                {
                    if (s.mBlocks[k][1] == j)
                    {
                        colorIdx ^= 1;
                        break;
                    }
                }

                if (diffCols.find(j) != diffCols.end())
                    o << ss.str() << oc::Color::Green;
                else
                    o << ss.str() << oc::Color::Default;

                if (lc.find(j) != lc.end())
                {
                    o << "1 ";
                }
                else
                {
                    o << (colorIdx ? ". " : "  ");
                }
                o << oc::Color::Default;
            }

            if (s.mWeights)
                o << "   " << (*s.mWeights)[i];


            if (s.mData2)
                o << "   " << (*s.mData2)[i];
            o << "\n";
        }

        return o;
    }

    void LDPC::View::init(LDPC& b)
    {
        //mH(b)
        mH = &b;
        mRowData.resize(b.rows());

        mWeightSets.resize(0);
        mWeightSets.resize(b.rowWeight() + 1, nullptr);
        mColNOMap.resize(b.cols());
        mColONMap.resize(b.cols());

        auto w = b.rowWeight();
        for (u64 i = 0; i < mRowData.size(); ++i)
        {
            auto& row = mRowData[i];
            row.mNOMap = i;
            row.mONMap = i;
            row.mPrevWeightNode = &row - 1;
            row.mNextWeightNode = &row + 1;
            row.mWeight = w;
        }

        mRowData.front().mPrevWeightNode = nullptr;
        mRowData.back().mNextWeightNode = nullptr;
        mWeightSets.back() = mRowData.data();

        for (u64 i = 0; i < mColNOMap.size(); ++i)
            mColNOMap[i] = mColONMap[i] = i;
    }

    void LDPC::View::swapRows(Idx& r0, Idx& r1)
    {
#ifdef LDPC_DEBUG
        assert(r0.mSrcIdx == rowIdx(r0.mViewIdx).mSrcIdx);
        assert(r1.mSrcIdx == rowIdx(r1.mViewIdx).mSrcIdx);
#endif

        std::swap(mRowData[r0.mViewIdx].mNOMap, mRowData[r1.mViewIdx].mNOMap);
        std::swap(mRowData[r0.mSrcIdx].mONMap, mRowData[r1.mSrcIdx].mONMap);
        std::swap(r0.mSrcIdx, r1.mSrcIdx);
    }

    void LDPC::View::swapCol(Idx& c0, Idx& c1)
    {
#ifdef LDPC_DEBUG
        assert(c0.mSrcIdx == colIdx(c0.mViewIdx).mSrcIdx);
        assert(c1.mSrcIdx == colIdx(c1.mViewIdx).mSrcIdx);
#endif

        std::swap(mColNOMap[c0.mViewIdx], mColNOMap[c1.mViewIdx]);
        std::swap(mColONMap[c0.mSrcIdx], mColONMap[c1.mSrcIdx]);
        std::swap(c0.mSrcIdx, c1.mSrcIdx);
    }

    std::pair<LDPC::Idx, u64> LDPC::View::popMinWeightRow()
    {
        Idx idx;
        for (u64 i = 1; i < mWeightSets.size(); ++i)
        {
            if (mWeightSets[i])
            {
                auto& front = mWeightSets[i];
                auto row = front;
                idx.mSrcIdx = row - mRowData.data();

                front = row->mNextWeightNode;

                if (front)
                    front->mPrevWeightNode = nullptr;

                idx.mViewIdx = mRowData[idx.mSrcIdx].mONMap;
                row->mWeight = 0;
                row->mNextWeightNode = nullptr;

                return { idx, i };
            }
        }

        //for (u64 i = 0; i < mBigWeightSets.size(); i++)
        //{
        //    if (mBigWeightSets[i].size())
        //    {
        //        auto iter = mBigWeightSets[i].begin();
        //        idx.mSrcIdx = *iter;
        //        idx.mIdx = mRowONMap[idx.mSrcIdx];
        //        mBigWeightSets[i].erase(iter);
        //        mRowWeights[idx.mSrcIdx] = 0;
        //        return { idx, i + mSmallWeightSets.size() };
        //    }
        //}

        throw RTE_LOC;
    }

    void LDPC::View::decRowWeight(const Idx& idx)
    {
        //assert(idx.mSrcIdx == rowIdx(idx.mIdx).mSrcIdx);
        auto& row = mRowData[idx.mSrcIdx];
        auto w = row.mWeight--;
#ifdef LDPC_DEBUG
        assert(w);
#endif

        auto prev = row.mPrevWeightNode;
        auto next = row.mNextWeightNode;

#ifdef LDPC_DEBUG
        assert(next == nullptr || next->mPrevWeightNode == &row);
        assert(prev == nullptr || prev->mNextWeightNode == &row);
#endif

        if (prev)
        {
            prev->mNextWeightNode = next;
        }
        else
        {
#ifdef LDPC_DEBUG
            assert(mWeightSets[w] == &row);
#endif
            mWeightSets[w] = next;
        }

        if (next)
        {
            next->mPrevWeightNode = prev;
        }

        row.mPrevWeightNode = nullptr;

        if (mWeightSets[w - 1])
        {
            mWeightSets[w - 1]->mPrevWeightNode = &row;
        }
        row.mNextWeightNode = mWeightSets[w - 1];
        mWeightSets[w - 1] = &row;

    }

    Matrix<u64> LDPC::View::applyPerm()const
    {
        Matrix<u64> newRows(mH->rows(), mH->rowWeight());
        applyPerm(newRows);
        return newRows;
        //mH->mRows = std::move(newRows);

        //std::vector<u64> newCols(mH->mColData.size());
        //std::vector<u64> colStartIdxs(mH->cols() + 1);
        //for (u64 i = 0, c = 0; i < mH->cols(); ++i)
        //{
        //    auto oIdx = mColNOMap[i];
        //    auto b = mH->mColStartIdxs[oIdx];
        //    auto e = mH->mColStartIdxs[oIdx + 1];
        //    colStartIdxs[i + 1] = colStartIdxs[i] + (e - b);

        //    while (b < e)
        //    {
        //        newCols[c++] = mRowData[mH->mColData[b++]].mONMap;
        //    }
        //}
        //mH->mColStartIdxs = std::move(colStartIdxs);
        //mH->mColData = std::move(newCols);
    }

    void LDPC::View::applyPerm(MatrixView<u64> newRows)const
    {
        Matrix<u64> temp;
        MatrixView<u64> src = mH->mRows;
        if (newRows.data() == src.data())
        {
            temp = src;
            src = temp;
        }

        for (u64 i = 0; i < mH->mRows.rows(); i++)
        {
            for (u64 j = 0; j < mH->mRows.cols(); ++j)
            {
                newRows(mRowData[i].mONMap, j) = mColONMap[src(i, j)];
            }
        }
    }

    LDPC::RowIter::RowIter(View& H, const Idx& i, u64 p)
        : mH(H)
        , mRow(H.mH->mRows[i.mSrcIdx])
        , mPos(p)
    { }


    LDPC::Idx LDPC::RowIter::operator*()
    {
        Idx idx;
        idx.mSrcIdx = mRow[mPos];
        idx.mViewIdx = mH.mColONMap[idx.mSrcIdx];
        return idx;
    }

    void LDPC::RowIter::operator++()
    {
        ++mPos;
    }
    LDPC::ColIter::ColIter(View& H, const Idx& i, u64 p)
        : mH(H)
        , mCol(H.mH->col(i.mSrcIdx))
        , mPos(p)
    { }

    void LDPC::ColIter::operator++()
    {
        ++mPos;
    }

    LDPC::Idx LDPC::ColIter::operator*()
    {
        Idx idx;
        idx.mSrcIdx = mCol[mPos];
        idx.mViewIdx = mH.mRowData[idx.mSrcIdx].mONMap;
        return idx;
    }

    u64 LDPC::ColIter::srcIdx()
    {
        return mCol[mPos];
    }

    LDPC::ColIter::operator bool()
    {
        return mPos < mCol.size();
    }
}

namespace std
{
    template<> struct hash<oc::LDPC::Idx>
    {
        std::size_t operator()(oc::LDPC::Idx const& i) const noexcept
        {
            return i.mViewIdx;
        }
    };
}

namespace osuCrypto
{

    void LDPC::insert(u64 numCols, MatrixView<u64> rows)
    {
        mNumCols = numCols;
        mRows = rows;
        auto numRows = rows.rows();
        auto numRows8 = (numRows / 8) * 8;

        auto size = rows.size();
        auto size8 = (size / 8) * 8;
        auto weight = rows.cols();

        mColData.resize(rows.size());

        bool hadData = mBackingColStartIdxs.size() > 0;
        mBackingColStartIdxs.resize(numCols + 2);

        if (hadData)
            memset(mBackingColStartIdxs.data(), 0, mBackingColStartIdxs.size() * sizeof(u64));



        mColStartIdxs = span<u64>(mBackingColStartIdxs.data() + 1, mBackingColStartIdxs.size() - 1);

        //for (auto& col : rows)
        //    ++mColStartIdxs[col + 1];
        auto rowsPtr = rows.data();
        auto counts = mColStartIdxs.data() + 1;

        for (u64 i8 = 0; i8 < size8; i8 += 8)
        {
            auto col0 = rowsPtr[i8 + 0];
            auto col1 = rowsPtr[i8 + 1];
            auto col2 = rowsPtr[i8 + 2];
            auto col3 = rowsPtr[i8 + 3];
            auto col4 = rowsPtr[i8 + 4];
            auto col5 = rowsPtr[i8 + 5];
            auto col6 = rowsPtr[i8 + 6];
            auto col7 = rowsPtr[i8 + 7];

            ++counts[col0];
            ++counts[col1];
            ++counts[col2];
            ++counts[col3];
            ++counts[col4];
            ++counts[col5];
            ++counts[col6];
            ++counts[col7];
        }


        for (u64 i = size8; i < size; ++i)
        {
            auto col = rowsPtr[i];
            ++counts[col];
        }


        for (u64 i = 1; i < mColStartIdxs.size(); ++i)
            mColStartIdxs[i] += mColStartIdxs[i - 1];

        auto ptr = rows.data();

        for (u64 i8 = 0; i8 < numRows8; i8 += 8)
        {
            auto ptr0 = ptr + weight * 0;
            auto ptr1 = ptr + weight * 1;
            auto ptr2 = ptr + weight * 2;
            auto ptr3 = ptr + weight * 3;
            auto ptr4 = ptr + weight * 4;
            auto ptr5 = ptr + weight * 5;
            auto ptr6 = ptr + weight * 6;
            auto ptr7 = ptr + weight * 7;
            ptr += 8 * weight;

            for (u64 j = 0; j < weight; ++j)
            {
                auto col0 = *ptr0++;
                auto col1 = *ptr1++;
                auto col2 = *ptr2++;
                auto col3 = *ptr3++;
                auto col4 = *ptr4++;
                auto col5 = *ptr5++;
                auto col6 = *ptr6++;
                auto col7 = *ptr7++;
                                
                auto p0 = mColStartIdxs[col0]++;
                auto p1 = mColStartIdxs[col1]++;
                auto p2 = mColStartIdxs[col2]++;
                auto p3 = mColStartIdxs[col3]++;
                auto p4 = mColStartIdxs[col4]++;
                auto p5 = mColStartIdxs[col5]++;
                auto p6 = mColStartIdxs[col6]++;
                auto p7 = mColStartIdxs[col7]++;

                mColData[p0] = i8 + 0;
                mColData[p1] = i8 + 1;
                mColData[p2] = i8 + 2;
                mColData[p3] = i8 + 3;
                mColData[p4] = i8 + 4;
                mColData[p5] = i8 + 5;
                mColData[p6] = i8 + 6;
                mColData[p7] = i8 + 7;
            }
        }

        for (u64 r = numRows8; r < numRows; ++r)
        {
            for (u64 j = 0; j < weight; ++j)
            {
                auto c = *ptr++;
                auto p = mColStartIdxs[c]++;
                mColData[p] = r;
            }
        }

        mColStartIdxs = span<u64>(mBackingColStartIdxs.data(), mBackingColStartIdxs.size() - 1);
    }

    void LDPC::blockTriangulate(
        std::vector<std::array<u64, 3>>& blocks,
        std::vector<u64>& rowPerm,
        std::vector<u64>& colPerm,
        bool verbose,
        bool stats,
        bool apply)
    {

        u64 n = cols();
        u64 m = rows();
        u64 k = 0;
        u64 i = 0;
        u64 v = n;

        blocks.resize(0);


        // temps
        std::vector<Idx> colSwaps(rowWeight());
        u64 numColSwaps{ 0 };

        // We are going to create a 'view' over the matrix.
        // At each iterations we will move some of the rows 
        // and columns in the view to the top/left. These 
        // moved rows will then be excluded from the view.
        // 
        //View H(*this);
        mView.init(*this);

        std::unique_ptr<Matrix<u64>> HH;
        if (verbose)
        {
            HH.reset(new Matrix<u64>(mRows));
        }

        //std::vector<double> avgs(rowWeight() + 1);
        //std::vector<u64> max(rowWeight() + 1);
        //u64 numSamples(0);

        while (i < m && v)
        {
            //numSamples++;
            //for (u64 j = 0; j < mView.mWeightSets.size(); ++j)
            //{
            //    avgs[j] += mView.mWeightSets[j].size();
            //    max[j] = std::max(max[j], mView.mSmallWeightSets[j].size());
            //}

            //for (u64 j = 0; j < mView.mBigWeightSets.size(); ++j)
            //{
            //    auto jj = j + mView.mSmallWeightSets.size();
            //    avgs[jj] += mView.mBigWeightSets[j].size();
            //    max[jj] = std::max(max[jj], mView.mBigWeightSets[j].size());
            //}

            if (mView.mWeightSets[0] == nullptr)
            {
                // If we don't have any rows with hamming
                // weight 0 then we will pick the row with 
                // minimim hamming weight and move it to the
                // top of the view.
                auto uu = mView.popMinWeightRow();
                auto u = uu.first;
                auto wi = uu.second;

                // move the min weight row u to row i.
                auto ii = mView.rowIdx(i);
                mView.swapRows(u, ii);

                if (verbose) {
                    std::cout << "wi " << wi << std::endl;
                    std::cout << "swapRow(" << i << ", " << u.mViewIdx << ")" << std::endl;
                }

                // For this newly moved row i, we need to move all the 
                // columns where this row has a non-zero value to the
                // left side of the view. 

                // c1 is the column defining the left side of the view
                auto c1 = (n - v);

                // rIter iterates the columns which have non-zero values for row ii.
                //auto rIter = mView.rowIterator(ii);
                //auto rIter = RowIter(mView, ii, 0);

                auto row = mRows[ii.mSrcIdx];

                // this set will collect all of the columns in the view. 
                //colSwaps.clear();
                numColSwaps = 0;
                for (u64 j = 0; j < rowWeight(); ++j)
                {
                    //auto c0 = colIdx[j];
                    //auto c0 = *rIter;

                    Idx c0;
                    c0.mSrcIdx = row[j];
                    c0.mViewIdx = mView.mColONMap[c0.mSrcIdx];

                    //++rIter;

                    // check if this column is inside the view.
                    if (c0.mViewIdx >= c1)
                    {
                        // add this column to the set of columns that we will move.
                        colSwaps[numColSwaps] = c0;
                        ++numColSwaps;
                        //colSwaps.push_back(c0);

                        if (verbose)
                            std::cout << "swapCol(" << c0.mViewIdx << ")" << std::endl;

                        // iterator over the rows for this column and decrement their row weight.
                        // we do this since we are about to move this column outside of the view.
                        //auto cIter = mView.colIterator(c0);
                        //auto cIter = ColIter(mView, c0, 0);
                        //auto col = mView.mH->col(c0.mSrcIdx);
                        auto b = &mColStartIdxs[c0.mSrcIdx];
                        auto e = b + 1;
                        span<u64> col(mColData.data() + *b, *e - *b);


                        //while (cIter)
                        for (u64 k = 0; k < col.size(); ++k)
                        {
                            // these a special case that this row is the u row which
                            // has already been decremented
                            if (col[k] != ii.mSrcIdx)
                            {
                                Idx idx;
                                idx.mSrcIdx = col[k];
                                idx.mViewIdx = mView.mRowData[col[k]].mONMap;
                                //mView.decRowWeight(idx);
                                {
                                    auto& row = mView.mRowData[idx.mSrcIdx];
                                    auto w = row.mWeight--;
                                    //assert(w);

                                    auto prev = row.mPrevWeightNode;
                                    auto next = row.mNextWeightNode;

                                    //assert(next == nullptr || next->mPrevWeightNode == &row);
                                    //assert(prev == nullptr || prev->mNextWeightNode == &row);

                                    if (GSL_LIKELY(prev))
                                    {
                                        prev->mNextWeightNode = next;
                                    }
                                    else
                                    {
                                        //assert(mWeightSets[w] == &row);
                                        mView.mWeightSets[w] = next;
                                    }

                                    if (next)
                                    {
                                        next->mPrevWeightNode = prev;
                                    }

                                    row.mPrevWeightNode = nullptr;

                                    if (mView.mWeightSets[w - 1])
                                    {
                                        mView.mWeightSets[w - 1]->mPrevWeightNode = &row;
                                    }
                                    row.mNextWeightNode = mView.mWeightSets[w - 1];
                                    mView.mWeightSets[w - 1] = &row;
                                }
                            }

                            //++cIter;
                        }
                    }
                }

                // now update the mappings so that these columns are
                // right before the view.
                while (numColSwaps)
                {
                    auto begin = colSwaps.begin();
                    auto end = colSwaps.begin() + numColSwaps;
                    auto back = end - 1;

                    auto cc = mView.colIdx(c1++);
                    auto sIter = std::find(begin, end, cc);
                    if (sIter != end)
                    {
                        std::swap(*sIter, *back);
                    }
                    else
                    {
                        mView.swapCol(cc, *back);
                    }

                    --numColSwaps;
                    //colSwaps.pop_back();
                }

                if (verbose)
                {
                    std::cout << "v " << (v - wi) << " = " << v << " - " << wi << std::endl;
                    std::cout << "i " << (i + 1) << " = " << i << " + 1" << std::endl;
                }

                // move the view right by wi.
                v = v - wi;

                // move the view down by 1
                ++i;
            }
            else
            {
                // in the case that we have some rows with
                // hamming weight 0, we will move all these
                // rows to the top of the view and remove them.


                auto rowPtr = mView.mWeightSets[0];
                mView.mWeightSets[0] = nullptr;

                std::vector<RowData*> rows;
                while (rowPtr)
                {
                    rows.push_back(rowPtr);
                    rowPtr = rowPtr->mNextWeightNode;
                }

                u64 dk = 0;

                // the top of the view where we will be moving
                // the rows too.
                u64 c1 = i;

                while (rows.size())
                {
                    ++dk;

                    // the actual input row index which we will 
                    // be swapping with.
                    auto c1SrcPtr = mView.mRowData.data() + mView.mRowData[c1].mNOMap;


                    // check that there isn't already a row
                    // that we want at the top.
                    auto sIter = std::find(rows.begin(), rows.end(), c1SrcPtr);
                    auto viewIdx = c1;

                    if (sIter == rows.end())
                    {
                        // if not then pick an arbitrary row
                        // that we will move to the top.
                        sIter = rows.begin();

                        auto inIdx = *sIter - mView.mRowData.data();
                        viewIdx = mView.mRowData[inIdx].mONMap;

                        Idx dest = mView.rowIdx(c1);
                        Idx src = { viewIdx, inIdx };// mView.rowSrcIdx((**sIter));

                        mView.swapRows(dest, src);
                    }

                    if (verbose)
                        std::cout << "rowSwap*(" << c1 << ", " << viewIdx << ")" << std::endl;
                    auto& row = **sIter;
                    row.mWeight = 0;
                    row.mNextWeightNode = nullptr;
                    row.mPrevWeightNode = nullptr;

                    rows.erase(sIter);
                    ++c1;
                }

                // recode that this the end of the block.
                blocks.push_back({ i + dk, n - v, dk });
                //dks.push_back(dk);

                if (verbose)
                {
                    std::cout << "RC " << blocks.back()[0] << " " << blocks.back()[1] << std::endl;
                    std::cout << "i " << (i + dk) << " = " << i << " + " << dk << std::endl;
                    std::cout << "k " << (k + 1) << " = " << k << " + 1" << std::endl;
                }

                i += dk;
                ++k;
            }

            if (verbose)
            {
                auto bb = blocks;
                bb.push_back({ i, n - v });
                Matrix<u64> W = mView.applyPerm();;


                std::vector<u64> weights(rows());
                std::vector<std::string> ids(rows());
                for (u64 i = 0; i < weights.size(); ++i)
                {
                    weights[i] = mView.mRowData[mView.mRowData[i].mNOMap].mWeight;
                    ids[i] = std::to_string(mView.mRowData[i].mNOMap) + " " + std::to_string(i);
                }

                std::cout << "\n" << diff(W, *HH, bb, cols(), &weights, &ids) << std::endl
                    << "=========================================\n"
                    << std::endl;

                *HH = std::move(W);
            }
        }

        //R.push_back(m);
        //C.push_back(n);

        rowPerm.resize(mView.mRowData.size());
        for (u64 i = 0; i < rowPerm.size(); ++i)
            rowPerm[i] = mView.mRowData[i].mONMap;
        //rowPerm = mView.mRowData.mONMap;
        colPerm = mView.mColONMap;

        if (apply)
            mView.applyPerm(mRows);

        //if (stats)
        //{

        //    for (u64 j = 0; j < avgs.size(); ++j)
        //    {
        //        std::cout << j << " avg  " << avgs[j] / numSamples << "  max  " << max[j] << std::endl;
        //    }
        //    std::array<u64, 3> prev = {};
        //    for (u64 j = 0; j < blocks.size(); ++j)
        //    {
        //        if (j == 50 && blocks.size() > 150)
        //        {
        //            std::cout << "..." << std::endl;
        //            j = blocks.size() - 50;
        //        }


        //        std::string dk;
        //        //if (i < dks.size())
        //        //    dk = std::to_string(dks[i]);

        //        std::cout << "RC[" << j << "] " << (blocks[j][0] - prev[0]) << " " << (blocks[j][1] - prev[1]) << "  ~   " << dk << std::endl;
        //        prev = blocks[j];
        //    }

        //    if (prev[0] != mRows.rows())
        //    {
        //        std::cout << "RC[" << blocks.size() << "] " << (mRows.rows() - prev[0]) << " " << (mNumCols - prev[1]) << "  ~   0" << std::endl;
        //    }
        //}

        //*this = applyPerm(mView.mRowONMap, mView.mColONMap);
    }

    void LDPC::validate()
    {
        for (u64 i = 0; i < rows(); ++i)
        {
            for (u64 j = 0; j < rowWeight(); ++j)
            {
                auto cIdx = mRows(i, j);
                auto c = col(cIdx);

                if (c.size() != 0 && std::find(c.begin(), c.end(), i) == c.end())
                    throw RTE_LOC;
            }
        }
    }




}