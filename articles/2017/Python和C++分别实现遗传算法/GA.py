import random
import math

class GA:

    #构造，gen_len为基因长度，size为种群大小，fit_func为适应度函数（接受一个Gen对象），mate_p和mutate_p分别是交配概率和变异概率
    def __init__(self,gen_len,size,fit_func,mate_p=0.1,mutate_p=0.1):
        if not isinstance(gen_len,int) or gen_len<=0:
            raise TypeError('<gen_len> should be a positive integer')
        if not isinstance(size,int) or size<=0:
            raise TypeError('<size> should be a positive integer')
        if not hasattr(fit_func,'__call__'):
            raise TypeError('<fit_func> should be a function')
        if not isinstance(mate_p,float) or mate_p<0 or mate_p>1:
            raise TypeError('<mate_p> should be a float in [0,1]')
        if not isinstance(mutate_p,float) or mutate_p<0 or mutate_p>1:
            raise TypeError('<mutate_p> should be a float in [0,1]')
        self.__gen_len=gen_len
        self.__size=size
        self.__fit_func=fit_func
        self.__mate_p=mate_p
        self.__mutate_p=mutate_p
        self.__gens=[Gen(gen_len) for i in range(size)]
        self.__best=None

    #产生新的一代
    def generate(self):
        for i in range(int(self.__size*self.__mate_p/2)):
            g1,g2=[self.__gens[random.randint(0,self.__size-1)] for i in range(2)]
            g3,g4=g1.mate(g2)
            self.__gens.append(g3)
            self.__gens.append(g4)
        for i in range(self.__size):
            if random.random()<self.__mutate_p:
                self.__gens.append(self.__gens[i].mutate())
        gen_fits=[(g,self.__fit_func(g)) for g in self.__gens]
        gen_fits.sort(key=lambda entry:entry[1],reverse=True)
        self.__best=gen_fits[0]
        self.__gens=[gt[0] for gt in gen_fits[:self.__size]]

    #获取当前最优解(基因,适应度)
    def best(self):
        return self.__best

class Gen:

    #构造，length是基因长度
    def __init__(self,length,bins=None):
        if not isinstance(length,int) or length<=0:
            raise TypeError('<length> should be a positive integer')
        self.__len=length
        if bins:
            if not isinstance(bins,list) or len(bins)!=length:
                raise TypeError('<bins> should be a %d-length list' % length)
            for b in bins:
                if b not in [0,1]:
                    raise ValueError('elements in <bins> should be 0 or 1')
            self.__bins=bins
        else:
            self.__bins=[0 if random.random()<0.5 else 1 for i in range(length)]

    #与另一条基因交配，产生两条新的基因
    def mate(self,gen):
        if not isinstance(gen,Gen):
            raise TypeError('<gen> should be an instance of Gen')
        if gen.__len!=self.__len:
            raise ValueError('<gen> has a different length')
        pos=random.randint(1,self.__len-1)
        bins_1=self.__bins[:pos]+gen.__bins[pos:]
        bins_2=gen.__bins[:pos]+self.__bins[pos:]
        return Gen(self.__len,bins_1),Gen(self.__len,bins_2)

    #变异，产生一条新的基因
    def mutate(self):
        pos=random.randint(0,self.__len-1)
        bins=self.__bins.copy()
        bins[pos]=1-bins[pos]
        return Gen(self.__len,bins)

    #返回二进制编码，是一个一维数组，每一个元素是0或1
    def bins(self):
        return self.__bins

    #把二进制编码中[start,end]的一段二进制映射到[x_min,x_max]上
    def decode(self,x_min,x_max,start=0,end=None):
        if not isinstance(x_min,(int,float)):
            raise TypeError('<x_min> should be a number')
        if not isinstance(x_max,(int,float)):
            raise TypeError('<x_max> should be a number')
        if not isinstance(start,int) or start<0 or start>=self.__len:
            raise TypeError('<start> should be an integer between [0,%d)' % self.__len)
        if end==None:
            end=self.__len-1
        if not isinstance(end,int) or end<start or end>=self.__len:
            raise TypeError('<end> should be an integer between [%d,%d)' % (start,self.__len))
        val=sum((self.__bins[start+i]<<i for i in range(end-start+1)))
        val_max=(1<<(end-start+1))-1
        return (x_max-x_min)*val/val_max+x_min

    def __str__(self):
        return ''.join([str(b) for b in self.__bins])