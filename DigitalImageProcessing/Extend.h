#pragma once
#include <vector>
#include <algorithm>
#include <queue>
#include "ImageUtil.h"


class WatershedPixel {
	static const int INIT = -1;	// ������ʼ��ͼ��
	static const int MASK = -2;	// ָʾ�����ص㽫������ÿ���㼶�ĳ�ʼֵ��
	static const int WSHED = 0;	// �������ص�����ĳ����ˮ��
	static const int FICTITIOUS = -3;	// �������ص�

	int x;	// ���ص�x����
	int y;	// ���ص�y����
	char height;	// ���ص�ĻҶ�ֵ
	int label;	// ���ڷ�ˮ���û�㷨�ı�ǩ
	int dist;	// ��������ʱ�õ��ľ���

	std::vector<WatershedPixel*> neighbours;	// ������������

public:
	WatershedPixel(int x, int y, char height) { // ���ص㹹�캯��
		this->x = x;
		this->y = y;
		this->height = height;
		label = INIT;
		dist = 0;
		neighbours.reserve(8);	// ���ڴ洢8��ͨ����������
	}
	WatershedPixel() { label = FICTITIOUS; } // �������ص㹹�캯��

	void addNeighbour(WatershedPixel* neighbour) { // �����������
		neighbours.push_back(neighbour);
	}
	std::vector<WatershedPixel*>& getNeighbours() { // ��ȡ��������
		return neighbours;
	}

	/* ��ȡ���ػҶ�ֵ������*/
	char getHeight() const { return height; }
	int getIntHeight() const { return (int)height & 0xff; }
	int getX() const { return x; }
	int getY() const { return y; }

	/* ���úͻ�ȡ��ǩ */
	void setLabel(int label) { this->label = label; }
	void setLabelToINIT() { label = INIT; }
	void setLabelToMASK() { label = MASK; }
	void setLabelToWSHED() { label = WSHED; }
	int getLabel() { return label; }

	/* �жϵ�ǰ��ǩ״̬ */
	bool isLabelINIT() { return label == INIT; }
	bool isLabelMASK() { return label == MASK; }
	bool isLabelWSHED() { return label == WSHED; }

	/* ���úͻ�ȡ���� */
	void setDistance(int distance) { dist = distance; }
	int getDistance() { return dist; }

	/* �ж��Ƿ�Ϊ�������� */
	bool isFICTITIOUS() { return label == FICTITIOUS; }

	/* �ж��Ƿ�������������Ϊ��ˮ�루���ڻ������ķ�ˮ�룩 */
	bool allNeighboursAreWSHED() {
		for (unsigned i = 0; i < neighbours.size(); i++) {
			WatershedPixel* r = neighbours.at(i);
			if (!r->isLabelWSHED()) return false;
		}
		return true;
	}
};

class WatershedStructure {
	std::vector<WatershedPixel*> watershedStructure;

public:
	WatershedStructure(byte* pixels, int width, int height) {	// �ṹ�幹�캯��

		watershedStructure.reserve(width * height);	 // �������ص�����Ԥ����ռ�
		/* ��ÿ�����ص���Ϣ����ṹ�� */
		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
				watershedStructure.push_back(new WatershedPixel(x, y, pixels[x + y * width]));

		/* ����������ص��8��ͨ�������� */
		for (int y = 0; y < height; ++y) {
			int offset = y * width;
			int topOffset = offset + width;
			int bottomOffset = offset - width;

			for (int x = 0; x < width; ++x) {
				int currentindex = x + y * width;	// ��ǰ���ص�����
				WatershedPixel* currentPixel = watershedStructure.at(currentindex);

				if (x - 1 >= 0) {
					currentPixel->addNeighbour(watershedStructure.at(currentindex - 1)); // ������
					if (y - 1 >= 0)	 // ���½�
						currentPixel->addNeighbour(watershedStructure.at(currentindex - 1 - width));
					if (y + 1 < height)	 // ���Ͻ�
						currentPixel->addNeighbour(watershedStructure.at(currentindex - 1 + width));
				}

				if (x + 1 < width) {
					currentPixel->addNeighbour(watershedStructure.at(currentindex + 1)); // ������
					if (y - 1 >= 0)	 // ���½�
						currentPixel->addNeighbour(watershedStructure.at(currentindex + 1 - width));
					if (y + 1 < height) // ���Ͻ�
						currentPixel->addNeighbour(watershedStructure.at(currentindex + 1 + width));
				}

				if (y - 1 >= 0) // ������
					currentPixel->addNeighbour(watershedStructure.at(currentindex - width));

				if (y + 1 < height)	 // ������
					currentPixel->addNeighbour(watershedStructure.at(currentindex + width));
			}
		}

		/* ���ݻҶ�ֵ�Խṹ���е��������ص��С����������� */
		std::sort(watershedStructure.begin(), watershedStructure.end(),
			[](WatershedPixel * pl, WatershedPixel * pr) { return pl->getIntHeight() < pr->getIntHeight(); });
	} // �������

	~WatershedStructure() {	// �ͷ���������ռ�ռ�
		while (!watershedStructure.empty()) {
			WatershedPixel* p = watershedStructure.back();
			delete p; p = NULL;
			watershedStructure.pop_back();
		}
	}

	int size() { return watershedStructure.size(); }	// �������ص�����

	WatershedPixel* at(int i) { return watershedStructure.at(i); }	// ����ĳ�����ص�
};

class WatershedAlgorithm {
	static const int HMIN = 0;	// ��С��
	static const int HMAX = 256;	// ����

public:
	void run(ImageUtil::IMGDATA* pSrc, const std::string& imgName) { // ��ˮ������㷨
		std::string inTmp;


		byte* img = new byte[pSrc->width * pSrc->height];
		for (int i = 1; i < pSrc->height; i++)
		{
			for (int j = 1; j < pSrc->width; j++)
			{
				img[i * pSrc->width + j] = (pow(pSrc->pImg[i * pSrc->width + j] - pSrc->pImg[i * pSrc->width + j - 1], 2)
					+ pow(pSrc->pImg[i * pSrc->width + j] - pSrc->pImg[(i - 1) * pSrc->width + j], 2))*0.5;
			}
		}


		/* ��ȡͼ����Ϣ */
		byte* pixels = img;
		int width = pSrc->width;
		int height = pSrc->height;


		/* Vincent and Soille ��ˮ���㷨��1991����һ��: �����ص����ṹ�岢���� */
		WatershedStructure  watershedStructure(pixels, width, height);

		/* Vincent and Soille ��ˮ���㷨��1991���ڶ���: ���飨ģ���û�� */
		/************************ ���飨��û����ʼ ****************************/
		std::queue<WatershedPixel*> pque;	// �洢���ص���ʱ����
		int curlab = 0;
		int heightIndex1 = 0;
		int heightIndex2 = 0;

		for (int h = HMIN; h < HMAX; ++h) { // h-1 ��� Geodesic SKIZ

			for (int pixelIndex = heightIndex1; pixelIndex < watershedStructure.size(); ++pixelIndex) {
				WatershedPixel* p = watershedStructure.at(pixelIndex);

				/* �����ص�λ�� h+1 �㣬�ݲ���������ѭ�� */
				if (p->getIntHeight() != h) { heightIndex1 = pixelIndex; break; }

				p->setLabelToMASK(); // ��Ǵ����ؽ�������

				std::vector<WatershedPixel*> neighbours = p->getNeighbours();
				for (unsigned i = 0; i < neighbours.size(); ++i) {
					WatershedPixel* q = neighbours.at(i);

					/* ��������ػ��ˮ���h����ڽ����ص���� */
					if (q->getLabel() >= 0) { p->setDistance(1); pque.push(p); break; }
				}
			}

			int curdist = 1;
			pque.push(new WatershedPixel());

			while (true) { // ��չ��ˮ���
				WatershedPixel* p = pque.front(); pque.pop();

				if (p->isFICTITIOUS())
					if (pque.empty()) { delete p; p = NULL; break; }
					else {
						pque.push(new WatershedPixel());
						curdist++;
						delete p; p = pque.front(); pque.pop();
					}

				std::vector<WatershedPixel*> neighbours = p->getNeighbours();
				for (unsigned i = 0; i < neighbours.size(); ++i) { // ͨ������ڽ���������� p
					WatershedPixel* q = neighbours.at(i);

					/* q����һ�����ڵ���ػ��ˮ�� */
					if ((q->getDistance() <= curdist) && (q->getLabel() >= 0)) {

						if (q->getLabel() > 0) {
							if (p->isLabelMASK())
								p->setLabel(q->getLabel());
							else if (p->getLabel() != q->getLabel())
								p->setLabelToWSHED();
						}
						else if (p->isLabelMASK())
							p->setLabelToWSHED();
					}
					else if (q->isLabelMASK() && (q->getDistance() == 0)) {
						q->setDistance(curdist + 1);
						pque.push(q);
					}
				} // �����ڽ����ص�forѭ��
			} // ��չ��ص�whileѭ��

			/* ��Ѱ������h�����µ���Сֵ */
			for (int pixelIndex = heightIndex2; pixelIndex < watershedStructure.size(); pixelIndex++) {
				WatershedPixel* p = watershedStructure.at(pixelIndex);

				/* �����ص�λ�� h+1 �㣬�ݲ���������ѭ�� */
				if (p->getIntHeight() != h) { heightIndex2 = pixelIndex; break; }

				p->setDistance(0); // ���þ���Ϊ0

				if (p->isLabelMASK()) { // ������λ������Сֵ����
					curlab++;
					p->setLabel(curlab);
					pque.push(p);

					while (!pque.empty()) {
						WatershedPixel* q = pque.front();
						pque.pop();

						std::vector<WatershedPixel*> neighbours = q->getNeighbours();

						for (unsigned i = 0; i < neighbours.size(); i++) { // ���p2����������
							WatershedPixel* r = neighbours.at(i);

							if (r->isLabelMASK()) { r->setLabel(curlab); pque.push(r); }
						}
					} // end while
				} // end if
			} // end for
		}
		/************************ ���飨��û������ ****************************/

		//cvCopyImage(pBW, pWS);

		/*
		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
				wsPixels[x + y * width] = (char)0;
		*/
		ImageUtil::outputImage(*pSrc, "bitmap/source_1.bmp");
		for (int pixelIndex = 0; pixelIndex < watershedStructure.size(); pixelIndex++) {
			WatershedPixel* p = watershedStructure.at(pixelIndex);

			if (p->isLabelWSHED() && !p->allNeighboursAreWSHED()) {
				img[p->getX() + p->getY()*width] = (char)255; // �ں�ɫ�����л��ư�ɫ��ˮ��
				pSrc->pImg[p->getX() + p->getY() * width] = 255;
				//grayPixels[p->getX() + p->getY()*width] = (char)255;	// �ڻҶ�ͼ�л��ư�ɫ��ˮ��
			}
		}
		ImageUtil::outputImage(*pSrc, "bitmap/source.bmp");

		memcpy(pSrc->pImg, img, width * height);
		ImageUtil::outputImage(*pSrc, imgName);

	}
};