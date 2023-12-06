class HuffmanCode(object):
    class __Nodes:
        def __init__(self, probability, symbol, left = None, right = None):  
            # probability of the symbol  
            self.probability = probability  
    
            # the symbol  
            self.symbol = symbol  
    
            # the left node  
            self.left = left  
    
            # the right node  
            self.right = right  
    
            # the tree direction (0 or 1)  
            self.code = '' 

    def __init__(self, bs):
        self.__bs = bs

    """ A supporting function in order to calculate the probabilities of symbols in specified data """  
    def CalculateProbability(self, the_data):  
        the_symbols = dict()  
        if the_symbols.get(the_data) == None:  
            the_symbols[the_data] = 1  
        else:   
            the_symbols[the_data] += 1       
        return the_symbols  
  
    """ A supporting function in order to print the codes of symbols by travelling a Huffman Tree """  
    the_codes = dict()  
  
    def CalculateCodes(self, node, value = ''):  
        # a huffman code for current node  
        newValue = value + str(node.code)  
    
        if(node.left):  
            self.CalculateCodes(node.left, newValue)  
        if(node.right):  
            self.CalculateCodes(node.right, newValue)  
    
        if(not node.left and not node.right):  
            self.the_codes[node.symbol] = newValue  
            
        return self.the_codes 


    def push(self, x):
        symbolWithProbs = self.CalculateProbability(x)  
        the_symbols = symbolWithProbs.keys()  
        the_probabilities = symbolWithProbs.values()  
        the_nodes = []  
        # converting symbols and probabilities into huffman tree nodes  
        for symbol in the_symbols:  
            the_nodes.append(self.__Nodes(symbolWithProbs.get(symbol), symbol))  
        
        while len(the_nodes) > 1:  
            # sorting all the nodes in ascending order based on their probability  
            the_nodes = sorted(the_nodes, key = lambda x: x.probability)  
            # for node in nodes:    
            #      print(node.symbol, node.prob)  
        
            # picking two smallest nodes  
            right = the_nodes[0]  
            left = the_nodes[1]  
        
            left.code = 0  
            right.code = 1  
        
            # combining the 2 smallest nodes to create new node  
            newNode = self.__Nodes(left.probability + right.probability, left.symbol + right.symbol, left, right)  
        
            the_nodes.remove(left)  
            the_nodes.remove(right)  
            the_nodes.append(newNode)
        huffmanEncoding = self.CalculateCodes(the_nodes[0])
        print(huffmanEncoding)  
        b = huffmanEncoding.get(huffmanEncoding.get('')).bit_length() - 1
        mask = (1 << b) - 1
        self.__bs.push_bits(huffmanEncoding & mask, b)

    def pop(self):
        treeHead = huffmanTree  
        decodedOutput = []  
        if self.__bs.pop_bits(1) == '1':  
            huffmanTree = huffmanTree.right     
        elif self.__bs.pop_bits(1) == '0':  
            huffmanTree = huffmanTree.left  
        try:  
            if huffmanTree.left.symbol == None and huffmanTree.right.symbol == None:  
                pass  
        except AttributeError:  
            decodedOutput.append(huffmanTree.symbol)  
            huffmanTree = treeHead  
        return decodedOutput