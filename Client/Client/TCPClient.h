#pragma once
struct Client_INFO {
	int Id;
	float x, y, z;
};

class TCPClient {
private:
	// ���� ���� ����
	SOCKET sock;
	// ������ ��ſ� ����� ����
	vector<BYTE> buffer;

	// ������ ������ Ŭ���̾�Ʈ�� ���� <���̵�,����>
	unordered_map<int, Client_INFO> client_infos;

	int my_Id;
public:
	TCPClient();
	~TCPClient();

	void Init();
	bool Logic();
	void Exit();

	bool SendData(char* data, int size);
	bool RecvData(char* data, int size);

	XMFLOAT3 GetPostion(int id);

	unordered_map<int, Client_INFO>& GetClientInfos();
	int GetMyId();
};