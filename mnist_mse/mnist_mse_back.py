import numpy as np
import matplotlib.pyplot as plt


def main():

    # 取得MNIST訓練集資料
    images, labels = loadLocal_MNIST('train-images-idx3-ubyte', 'train-labels-idx1-ubyte')
    images = images / 255

    # print(labels)
    # print(images[0])
    #plt.imshow(images[0].reshape(28, 28))
    # plt.show()

    #plt.figure(figsize=(10, 10))
    # for i in range(25):
    #    plt.subplot(5, 5, i+1)
    #    plt.xticks([])
    #    plt.yticks([])
    #    plt.imshow(images[i].reshape(28, 28))
    # plt.show()

    # 指定樣本使用最大數量
    datasize = 1000 #len(images)
    
    epochs = 1000
    learning_rate = 0.001
    
    num_input = 28 * 28
    num_hidden1 = 100
    num_classes = 10

    # Graph input
    # 設計輸入端神經元樣式
    X = np.empty([1, num_input], dtype=np.float32)
    # 設計輸出端神經元樣式
    Y = np.empty([1, num_classes], dtype=np.float32)

    # Store layers weight & bias
    # 設計神經元連線間的權重值，並以亂數隨機分布權重值
    weights = {
        'h1': variable([num_input, num_hidden1], 10),
        'out': variable([num_hidden1, num_classes], 10)}

    # biases = {'out': variable([num_classes], 10)}
    # biases['out'] = np.zeros(biases['out'].shape)

    for e in range(epochs):
        
        bingo = 0

        for i in range(datasize):

            # 將指定圖片(28x28=768)與指定答案([1,0,0,0,0,0,0,0,0,0])放入
            X = images[i]
            Y = setLabel(labels[i], num_classes)

            # 將目前的指定圖片，流經目前的權重值，來預測答案
            p_a = np.sum(weights['h1'].transpose() * X, axis = 1)
            predicted = weights['out'].transpose() * p_a

            t_t = Y
            a_t = softmax(np.sum(predicted, axis = 1))
            e_t = t_t - a_t

            # 計算準確度
            if ( np.where(t_t == np.max(t_t)) == np.where(a_t == np.max(a_t)) ):
                bingo += 1
                if (i == datasize -1):
                    print('epoch:', e + 1, 'i:', i + 1, 'bingo:', bingo, 'accuracy:', (bingo + 1) / (i + 1) )
            

            # 修正權重[OUT]
            for index in range(len(e_t)):
                weights['out'].transpose()[index] = weights['out'].transpose()[index] + (2 * learning_rate * e_t[index] * p_a)
            
            # 修正權重[H1]
            for index in range(len(e_t)):

                t_t1 = softmax(np.sum(weights['out'], axis = 1))
                a_t1 = softmax(np.sum(weights['h1'], axis = 0))
                e_t1 = t_t1 - a_t1

                for h in range(len(e_t1)):
                    weights['h1'].transpose()[h] = weights['h1'].transpose()[h] + (2 * learning_rate * e_t1[h] * X)




                    
                    # print('Dataset Index:', i, 'modify out:', index, 'modify h1:', h)
            
        print('Epochs:', e + 1)
    
    #bingo = 0
    #for target in range(datasize):
    #    
    #    X = images[target]
    #    Y = setLabel(labels[target], num_classes)
    #    predicted = weights['out'].transpose() * X
    #    _softmax = softmax(np.sum(predicted, axis = 1))
    #    result = np.where(_softmax == np.max(_softmax))
    #
    #    if ( labels[target][0] == result[0][0] ):
    #        bingo += 1
    #        print('target:', target, 'bingo:', bingo, 'accuracy:', (bingo + 1) / (target + 1) )

    

    # 測試資料
    print('-----FIRST DATA-----')
    for target in range(20):
        predicted = np.sum(weights['h1'].transpose() * images[target], axis = 1).transpose() * weights['out'].transpose()
        _softmax = softmax(np.sum(predicted, axis = 1))
        result = np.where(_softmax == np.max(_softmax))
        print('Label:', labels[target], ', Predicted:', result[0])


    print('-----LAST DATA-----')
    for target in range(20):
        predicted = np.sum(weights['h1'].transpose() * images[len(images) - target -1], axis = 1).transpose() * weights['out'].transpose()
        _softmax = softmax(np.sum(predicted, axis = 1))
        result = np.where(_softmax == np.max(_softmax))
        print('Label:', labels[len(images) - target -1], ', Predicted:', result[0])

    print('-----FINISH-----')


def softmax(x):
    exp_x = np.exp(x - np.max(x))
    softmax_x = exp_x / exp_x.sum(axis = 0)
    return softmax_x

def variable(shape, seed=1):
    shape = np.array(shape)
    np.random.seed(seed)
    if (shape.size == 1):
        return np.random.rand(shape[0])
    if (shape.size == 2):
        return np.random.rand(shape[0], shape[1])


def setLabel(value, num_classes):
    # result = np.empty(num_classes, dtype=np.float32)
    result = np.zeros(num_classes)
    result[value[0]] = 1.0
    return result


def loadLocal_MNIST(images_path, labels_path):
    print(images_path, labels_path)
    imageBytes = np.fromfile(images_path, dtype=np.uint8)
    labelBytes = np.fromfile(labels_path, dtype=np.uint8)

    magic_number = imageBytes[0:4]
    image_size = int.from_bytes(imageBytes[4:8], byteorder='big')
    rows = int.from_bytes(imageBytes[8:12], byteorder='big')
    columns = int.from_bytes(imageBytes[12:16], byteorder='big')

    images = []
    labels = []

    index = 0
    image_start = 16
    image_length = 28 * 28

    label_start = 8
    label_length = 1

    for i in range(image_size):
        _pixels = imageBytes[image_start + (image_length * i) : image_start + (image_length * i) + image_length]
        images.append(_pixels)

        _labels = labelBytes[label_start + (label_length * i) : label_start + (label_length * i) + label_length]
        labels.append(_labels)

        #print('labels', _labels)
        # plt.cla()
        #plt.imshow(_pixels.reshape(rows, columns))
        # plt.show()

    return np.array(images), np.array(labels)

main()
