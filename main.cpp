#include <iostream>
#include <string>
#include <fstream>

using namespace std;

//Link do repositório: https://github.com/GMP-P/B-tree

class MyBTree {
public:
    // selecionador de ordem da árvore
    static const int M = 1;
    static const int MIN = M;
    static const int MAX = 2 * M;

    class MyNode { // classe nodo dentro da árvore
    public:
        int keys[MAX + 1];
        int values[MAX + 1];

        MyNode* children[MAX + 2]; // filhos com espaço extra por causa do overflow temporário

        int size;
        bool leaf;

        MyNode(bool isLeaf) { // criação de página vazia
            size = 0;
            leaf = isLeaf;

            for (int i = 0; i < MAX + 2; i++)
                children[i] = NULL;
        }

        bool IsLeaf() {
            return leaf;
        }
    };

    MyBTree() {
        root = NULL;
        height = -1;
        size = 0;
    }

    bool IsEmpty() {
        return root == NULL;
    }

    int Size() {
        return size;
    }

    int Height() {
        return height;
    }

    void Clear() {
        Clear(root);
        root = NULL;
        size = 0;
        height = -1;
    }

    int Search(int key) { // procura pela chave
        int value; // variável pra guardar o valor da chave

        if (Search(root, key, value))
            return value;

        return -1;
    }

    bool Contains(int key) { // mesma lógica do search, porém não importa o valor
        int value;
        return Search(root, key, value);
    }

    bool Insert(int key, int value) {
        cout << "--- Inserting: " << key << endl;

        if (Contains(key)) {
            cout << "Chave ja existe. Insercao cancelada." << endl;
            return false;
        }

        if (root == NULL) {
            root = new MyNode(true);
            root->keys[0] = key;
            root->values[0] = value;
            root->size = 1;

            size++;
            height = 0;

            return true;
        }
        // variáveis para possível split
        int upKey;
        int upValue;
        MyNode* newChild = NULL;

        bool splitRoot = Insert(root, key, value, upKey, upValue, newChild);

        size++;

        if (splitRoot) { // tratativa do split
            MyNode* oldRoot = root;

            root = new MyNode(false); // cria nova raiz não folha
            root->keys[0] = upKey;
            root->values[0] = upValue; // coloca na nova raiz a chave promovida
            root->children[0] = oldRoot;
            root->children[1] = newChild; // nova raiz aponta para a raiz antiga e para a página criada no split
            root->size = 1;

            height++;
        }

        return true;
    }

    bool Remove(int key) {
        cout << "--- Removing: " << key << endl;

        if (root == NULL)
            return false;

        bool removed = Remove(root, key);

        if (!removed) {
            cout << "Chave nao encontrada. Remocao cancelada." << endl;
            return false;
        }

        size--;

        if (root->size == 0) { // tratativa quando a raiz fica vazia, por causa de merge
            MyNode* oldRoot = root;

            if (root->leaf) { // se era folha e ficou vazia, a árvore fica vazia
                root = NULL;
                height = -1;
            }
            else { // se tinha filho, esse vira a raiz
                root = root->children[0];
                height--;
            }

            delete oldRoot;
        }

        return true;
    }

    void Print() {
        if (root == NULL) {
            cout << "Arvore vazia\n";
            return;
        }

        Print(root, "", true, "ROOT");
    }

    void PreOrder() {
        cout << "Pre-ordem: ";
        PreOrder(root);
        cout << endl;
    }

    void InOrder() {
        cout << "Em ordem: ";
        InOrder(root);
        cout << endl;
    }

    void PostOrder() {
        cout << "Pos-ordem: ";
        PostOrder(root);
        cout << endl;
    }
    void GenerateDotFile(string filename) {
        ofstream file(filename);

        if (!file.is_open()) {
            cout << "Erro ao criar arquivo DOT." << endl;
            return;
        }

        file << "digraph BTree {\n";
        file << "    node [shape=record];\n";
        file << "    rankdir=TB;\n\n";

        int counter = 0;

        if (root != NULL)
            GenerateDotPreOrder(root, file, counter);

        file << "}\n";

        file.close();

        cout << "Arquivo DOT gerado: " << filename << endl;
    }

private:
    MyNode* root;
    int height;
    int size;

    void Clear(MyNode* node) {
        if (node == NULL)
            return;

        if (!node->leaf) { // limpa primeiro os filhos recursivamente
            for (int i = 0; i <= node->size; i++)
                Clear(node->children[i]);
        }

        delete node;
    }

    bool Search(MyNode* node, int key, int& value) {
        if (node == NULL)
            return false;

        int i = 0;

        while (i < node->size && key > node->keys[i]) // // avança dentro da página enquanto a chave procurada for maior que a chave atual
            i++;

        if (i < node->size && key == node->keys[i]) { // se encontra, retorna true e copia o valor da chave pra value
            value = node->values[i];
            return true;
        }

        if (node->leaf) // se chegou numa folha e ainda não achou, a chave não existe
            return false;

        return Search(node->children[i], key, value); // se não, desce para a próxima página
    }

    bool Insert(MyNode* node, int key, int value, int& upKey, int& upValue, MyNode*& newChild) {
        int i = 0;

        if (node->leaf) { // se o nó é folha, insere alí mesmo
            while (i < node->size && key > node->keys[i])
                i++;

            InsertInNode(node, i, key, value, NULL); // na posição correta. como é folha, não tem filho direito
        }
        else { // se não for folha, precisa descer para o filho correto
            while (i < node->size && key > node->keys[i]) // procura o filho correto (mesma lógica do search)
                i++;


            int promotedKey;
            int promotedValue;
            MyNode* promotedChild = NULL; // variáveis para o possível split do filho

            bool splitChild = Insert(node->children[i], key, value, promotedKey, promotedValue, promotedChild); // chama a inserção recursivamente

            if (!splitChild) // se o filho não dividiu, página não precisa fazer nada
                return false;

            InsertInNode(node, i, promotedKey, promotedValue, promotedChild); // se o filho dividiu, a chave promovida entra nessa página, e o novo filho criado no split tmb é conectado
        }

        if (node->size <= MAX) // verifica se a página precisa dividir
            return false;

        Split(node, upKey, upValue, newChild); // se precisa, a chave do meio sobe pro pai
        return true;
    }

    void InsertInNode(MyNode* node, int pos, int key, int value, MyNode* rightChild) {
        for (int j = node->size; j > pos; j--) { // desloca as chaves para a direita para abrir espaço
            node->keys[j] = node->keys[j - 1];
            node->values[j] = node->values[j - 1];
        }

        if (!node->leaf) { // caso não seja folha, desloca os filhos tmb
            for (int j = node->size + 1; j > pos + 1; j--)
                node->children[j] = node->children[j - 1];

            node->children[pos + 1] = rightChild; // coloca o novo filho à direita da chave inserida
        }

        node->keys[pos] = key;
        node->values[pos] = value;
        node->size++; // insere chave e valor, além de aumentar a quantidade de chaves na página
    }

    void Split(MyNode* node, int& upKey, int& upValue, MyNode*& newChild) {
        int middle = MIN; // define o meio a partir do MIN. Ex.: MIN = 1, [0, *1, 2]. MIN = 2, [0, 1, *2, 3, 4]

        upKey = node->keys[middle];
        upValue = node->values[middle];

        newChild = new MyNode(node->leaf); // cria página nova do mesmo tipo da atual
        newChild->size = MIN;

        for (int j = 0; j < MIN; j++) { // copia as chaves da direita pra nova página
            newChild->keys[j] = node->keys[middle + 1 + j];
            newChild->values[j] = node->values[middle + 1 + j];
        }

        if (!node->leaf) { // copia os filhos da direita tmb caso não seja folha
            for (int j = 0; j <= MIN; j++)
                newChild->children[j] = node->children[middle + 1 + j];
        }

        node->size = MIN; // página da esquerda passa a ficar só com as chaves da esquerda
    }

    bool Remove(MyNode* node, int key) {
        int i = 0;

        while (i < node->size && key > node->keys[i])
            i++;

        if (i < node->size && key == node->keys[i]) { // se encontrou a chave na página atual
            if (node->leaf) { // se tá na folha, remove a chave direto
                RemoveKey(node, i);
                return true;
            }
            else {
                MyNode* pred = MaxNode(node->children[i]); // se a chave está em um nó interno, busca o predecessor, ou seja, a maior chave da subárvore esquerda

                int predKey = pred->keys[pred->size - 1];
                int predValue = pred->values[pred->size - 1]; // pega a última chave da folha encontrada

                node->keys[i] = predKey;
                node->values[i] = predValue; // substitui a chave removida pelo predecessor

                Remove(node->children[i], predKey); // e remove ele da folha original

                if (node->children[i]->size < MIN) // se ficou com menos chaves que o mínimo, corrige o underflow
                    FixChild(node, i);

                return true;
            }
        }

        if (node->leaf) // se a página atual é folha e não achou, então não existe
            return false;

        bool removed = Remove(node->children[i], key); // se não for folha, continua procurando no filho correto

        if (removed && node->children[i]->size < MIN) // se removeu e o filho deu underflow, corrige
            FixChild(node, i);

        return removed; // retorna o bool das chamadas recursivas
    }

    void RemoveKey(MyNode* node, int pos) { // remove a chave de uma página
        for (int j = pos; j < node->size - 1; j++) { // desloca as chaves depois da posição removida pra esquerda
            node->keys[j] = node->keys[j + 1];
            node->values[j] = node->values[j + 1];
        }

        node->size--;
    }

    MyNode* MaxNode(MyNode* node) { // busca a página que contém a maior chave de uma subárvore
        if (node->leaf) // se chegou numa folha, é alí que tá
            return node;

        return MaxNode(node->children[node->size]);// se não, vai indo pelo filho mais à direita
    }

    void FixChild(MyNode* parent, int index) { // corrige underflow em um filho
        if (index > 0 && parent->children[index - 1]->size > MIN) { // se ele tem irmão esquerdo com mais do q o mínimo, pega emprestado
            BorrowFromLeft(parent, index);
        }
        else if (index < parent->size && parent->children[index + 1]->size > MIN) { // se não tem e o da direita tem, pega emprestado dele
            BorrowFromRight(parent, index);
        }
        else { // se nenhum tem, faz merge
            if (index > 0)
                Merge(parent, index - 1);
            else
                Merge(parent, index);
        }
    }

    void BorrowFromLeft(MyNode* parent, int index) { // pega uma chave emprestada do irmão esquerdo
        MyNode* child = parent->children[index];
        MyNode* left = parent->children[index - 1]; // guarda filho com underflow e o irmão esquerdo

        for (int j = child->size; j > 0; j--) { // abre espaço no começo do filho, pq a chave que desce do pai precisa entrar na primeira posição
            child->keys[j] = child->keys[j - 1];
            child->values[j] = child->values[j - 1];
        }

        if (!child->leaf) { // se não é folha, tmb move os filhos. último filho do irmão esquerdo passa a ser o primeiro filho do filho corrigido
            for (int j = child->size + 1; j > 0; j--)
                child->children[j] = child->children[j - 1];

            child->children[0] = left->children[left->size];
        }

        child->keys[0] = parent->keys[index - 1];
        child->values[0] = parent->values[index - 1];
        child->size++; // desce a chave do pai para o filho

        parent->keys[index - 1] = left->keys[left->size - 1];
        parent->values[index - 1] = left->values[left->size - 1]; // maior chave do irmão esquerdo sobe pro pai

        left->size--;
    }

    void BorrowFromRight(MyNode* parent, int index) { // pega uma chave emprestada do irmão direito
        MyNode* child = parent->children[index];
        MyNode* right = parent->children[index + 1]; // guarda filho com underflow e o irmão direito

        child->keys[child->size] = parent->keys[index];
        child->values[child->size] = parent->values[index]; // chave do pai desce para o final do filho

        if (!child->leaf) // se não é folha, o primeiro filho do irmão direito passa para o filho corrigido
            child->children[child->size + 1] = right->children[0];

        child->size++;

        parent->keys[index] = right->keys[0];
        parent->values[index] = right->values[0]; // menor chave do irmão direito sobe pro pai 

        for (int j = 0; j < right->size - 1; j++) { // remove a primeira chave do irmão direito, e desloca o resto pra esquerda
            right->keys[j] = right->keys[j + 1];
            right->values[j] = right->values[j + 1];
        }

        if (!right->leaf) { // se o irmão direito não é folha, também desloca os seus filhos pra esquerda
            for (int j = 0; j < right->size; j++)
                right->children[j] = right->children[j + 1];
        }

        right->size--;
    }

    void Merge(MyNode* parent, int index) {
        MyNode* left = parent->children[index];
        MyNode* right = parent->children[index + 1]; // pega o filho esquerdo e o filho direito que vão ser unidos

        int pos = left->size; // guarda a posição onde a chave do pai vai entrar no filho esquerdo

        left->keys[pos] = parent->keys[index];
        left->values[pos] = parent->values[index];
        left->size++; // chave do pai desce pro filho esquerdo

        for (int j = 0; j < right->size; j++) { // copia as chaves do irmão direito para dentro do esquerdo
            left->keys[left->size + j] = right->keys[j];
            left->values[left->size + j] = right->values[j];
        }

        if (!left->leaf) { // se não for folha, copia tmb os filhos do irmão direito
            for (int j = 0; j <= right->size; j++)
                left->children[left->size + j] = right->children[j];
        }

        left->size += right->size;

        for (int j = index; j < parent->size - 1; j++) { // remove do pai a chave que desceu pra merge 
            parent->keys[j] = parent->keys[j + 1];
            parent->values[j] = parent->values[j + 1];
        }

        for (int j = index + 1; j < parent->size; j++) // remove tmb o ponteiro pro filho direito, já que teve merge com o esquerdo
            parent->children[j] = parent->children[j + 1];

        parent->size--;

        delete right;
    }

    void Print(MyNode* node, string prefix, bool isLast, string label) {
        if (node == NULL)
            return;

        cout << prefix; // imprime a identação atual

        if (label == "ROOT")
            cout << "ROOT ";
        else
            cout << (isLast ? "`-- " : "|-- ") << label << " "; // |-- é para filhos que ainda tem irmãos dps, `-- é para o último filho

        cout << "[";

        for (int i = 0; i < node->size; i++) { // imprime todas as chaves da página
            cout << node->keys[i];

            if (i < node->size - 1)
                cout << " | ";
        }

        cout << "]\n";

        if (!node->leaf) { // se não é folha, imprime os filhos
            for (int i = 0; i <= node->size; i++) {
                string newPrefix = prefix;

                if (label == "ROOT")
                    newPrefix += "     ";
                else
                    newPrefix += isLast ? "    " : "|   ";

                Print(node->children[i], newPrefix, i == node->size, "F" + to_string(i));
            }
        }
    }
    void PreOrder(MyNode* node) {
        if (node == NULL)
            return;

        // Primeiro visita a página atual
        cout << "[";

        for (int i = 0; i < node->size; i++) {
            cout << node->keys[i];

            if (i < node->size - 1)
                cout << " | ";
        }

        cout << "] ";

        // Depois visita os filhos
        if (!node->leaf) {
            for (int i = 0; i <= node->size; i++)
                PreOrder(node->children[i]);
        }
    }
    void InOrder(MyNode* node) {
        if (node == NULL)
            return;

        if (node->leaf) {
            for (int i = 0; i < node->size; i++)
                cout << node->keys[i] << " ";

            return;
        }

        // Em B-Tree:
        // filho0, chave0, filho1, chave1, filho2...
        for (int i = 0; i < node->size; i++) {
            InOrder(node->children[i]);
            cout << node->keys[i] << " ";
        }

        InOrder(node->children[node->size]);
    }
    void PostOrder(MyNode* node) {
        if (node == NULL)
            return;

        // Primeiro visita os filhos
        if (!node->leaf) {
            for (int i = 0; i <= node->size; i++)
                PostOrder(node->children[i]);
        }

        // Depois visita a página atual
        cout << "[";

        for (int i = 0; i < node->size; i++) {
            cout << node->keys[i];

            if (i < node->size - 1)
                cout << " | ";
        }

        cout << "] ";
    }
    int GenerateDotPreOrder(MyNode* node, ofstream& file, int& counter) { // gera o arquivo DOT usando a mesma lógica do pré-ordem, ou seja, visita a página atual e depois seus filhos
        if (node == NULL)
            return -1;

        int currentId = counter;
        counter++;

        file << "    node" << currentId << " [label=\"";

        for (int i = 0; i < node->size; i++) {
            file << node->keys[i];

            if (i < node->size - 1)
                file << " | ";
        }

        file << "\"];\n";

        if (!node->leaf) {
            for (int i = 0; i <= node->size; i++) {
                int childId = GenerateDotPreOrder(node->children[i], file, counter);

                if (childId != -1)
                    file << "    node" << currentId << " -> node" << childId << ";\n";
            }
        }

        return currentId;
    }
};

int main() {
    MyBTree* t1 = new MyBTree();
    t1->Insert(6, 6);
    t1->GenerateDotFile("dotFiles/Arvore1_AntesInsercaoSimples.dot");

    t1->Insert(9, 9);

    t1->GenerateDotFile("dotFiles/Arvore1_DepoisInsercaoSimples.dot");

    t1->Clear();
    delete t1;


    MyBTree* t2 = new MyBTree();

    t2->Insert(6, 6);
    t2->Insert(9, 9);

    t2->GenerateDotFile("dotFiles/Arvore2_AntesSplitRaiz.dot");

    t2->Insert(3, 3);

    t2->GenerateDotFile("dotFiles/Arvore2_DepoisSplitRaiz.dot");

    t2->Clear();
    delete t2;


    MyBTree* t3 = new MyBTree();

    t3->Insert(6, 6);
    t3->Insert(9, 9);
    t3->Insert(3, 3);

    t3->GenerateDotFile("dotFiles/Arvore3_AntesInsercaoFilhoSemSplit.dot");

    t3->Insert(7, 7);

    t3->GenerateDotFile("dotFiles/Arvore3_DepoisInsercaoFilhoSemSplit.dot");

    t3->Clear();
    delete t3;


    MyBTree* t4 = new MyBTree();

    t4->Insert(6, 6);
    t4->Insert(9, 9);
    t4->Insert(3, 3);
    t4->Insert(7, 7);

    t4->GenerateDotFile("dotFiles/Arvore4_AntesSplitFilho.dot");

    t4->Insert(13, 13);

    t4->GenerateDotFile("dotFiles/Arvore4_DepoisSplitFilho.dot");

    t4->Clear();
    delete t4;


    MyBTree* t5 = new MyBTree();

    t5->Insert(6, 6);
    t5->Insert(9, 9);
    t5->Insert(3, 3);
    t5->Insert(7, 7);
    t5->Insert(13, 13);
    t5->Insert(1, 1);

    t5->GenerateDotFile("dotFiles/Arvore5_AntesSplitRecursivo.dot");

    t5->Insert(4, 4);

    t5->GenerateDotFile("dotFiles/Arvore5_DepoisSplitRecursivo.dot");

    t5->Clear();
    delete t5;


    MyBTree* t6 = new MyBTree();

    t6->Insert(6, 6);
    t6->Insert(9, 9);
    t6->Insert(3, 3);
    t6->Insert(7, 7);
    t6->Insert(13, 13);
    t6->Insert(1, 1);

    t6->GenerateDotFile("dotFiles/Arvore6_AntesRemoveFolhaSemUnderflow.dot");

    t6->Remove(3);

    t6->GenerateDotFile("dotFiles/Arvore6_DepoisRemoveFolhaSemUnderflow.dot");

    t6->Clear();
    delete t6;


    MyBTree* t7 = new MyBTree();

    t7->Insert(6, 6);
    t7->Insert(9, 9);
    t7->Insert(3, 3);
    t7->Insert(7, 7);
    t7->Insert(13, 13);
    t7->Insert(1, 1);

    t7->GenerateDotFile("dotFiles/Arvore7_AntesRemoveNoInterno.dot");

    t7->Remove(6);

    t7->GenerateDotFile("dotFiles/Arvore7_DepoisRemoveNoInterno.dot");

    t7->Clear();
    delete t7;


    MyBTree* t8 = new MyBTree();

    t8->Insert(6, 6);
    t8->Insert(9, 9);
    t8->Insert(1, 1);
    t8->Insert(3, 3);
    t8->Insert(7, 7);
    t8->Insert(13, 13);

    t8->GenerateDotFile("dotFiles/Arvore8_AntesEmprestimoEsquerdo.dot");

    t8->Remove(7);

    t8->GenerateDotFile("dotFiles/Arvore8_DepoisEmprestimoEsquerdo.dot");

    t8->Clear();
    delete t8;


    MyBTree* t9 = new MyBTree();

    t9->Insert(6, 6);
    t9->Insert(9, 9);
    t9->Insert(1, 1);
    t9->Insert(7, 7);
    t9->Insert(13, 13);
    t9->Insert(15, 15);

    t9->GenerateDotFile("dotFiles/Arvore9_AntesEmprestimoDireito.dot");

    t9->Remove(7);

    t9->GenerateDotFile("dotFiles/Arvore9_DepoisEmprestimoDireito.dot");

    t9->Clear();
    delete t9;


    MyBTree* t10 = new MyBTree();

    t10->Insert(6, 6);
    t10->Insert(1, 1);
    t10->Insert(9, 9);

    t10->GenerateDotFile("dotFiles/Arvore10_AntesMergeIrmaoPobre.dot");

    t10->Remove(1);

    t10->GenerateDotFile("dotFiles/Arvore10_DepoisMergeIrmaoPobre.dot");

    t10->Clear();
    delete t10;

    return 0;
}