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
    
    epochs = 50
    learning_rate = 0.001
    
    num_input = 28 * 28
    num_classes = 10

    # Graph input
    # 設計輸入端神經元樣式
    X = np.empty([1, num_input], dtype=np.float32)
    # 設計輸出端神經元樣式
    Y = np.empty([1, num_classes], dtype=np.float32)

    # Store layers weight & bias
    # 設計神經元連線間的權重值，並以亂數隨機分布權重值
    weights = {'out': variable([num_input, num_classes], 10)}

    # biases = {'out': variable([num_classes], 10)}
    # biases['out'] = np.zeros(biases['out'].shape)

    for e in range(epochs):
        
        bingo = 0

        for i in range(datasize):

            # 將指定圖片(28x28=768)與指定答案([1,0,0,0,0,0,0,0,0,0])放入
            X = images[i]
            Y = setLabel(labels[i], num_classes)

            # 將目前的指定圖片，流經目前的權重值，來預測答案
            predicted = weights['out'].transpose() * X

            t_t = Y
            a_t = np.sum(predicted, axis=1)
            # 取得Error間距
            e_t = t_t - a_t

            # 計算準確度
            if ( np.where(t_t == np.max(t_t)) == np.where(a_t == np.max(a_t)) ):
                bingo += 1
                print('epoch:', e + 1, 'i:', i + 1, 'bingo:', bingo, 'accuracy:', (bingo + 1) / (i + 1) )
                
            # 修正權重
            for index in range(len(e_t)):
                weights['out'].transpose()[index] = weights['out'].transpose()[index] + (2 * learning_rate * e_t[index] * X)
            
        print('Epochs:', e)
    
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
        predicted = weights['out'].transpose() * images[target]
        _softmax = softmax(np.sum(predicted, axis = 1))
        result = np.where(_softmax == np.max(_softmax))
        print('Label:', labels[target], ', Predicted:', result[0])

    print('-----LAST DATA-----')
    for target in range(20):
        predicted = weights['out'].transpose() * images[len(images) - target -1]
        _softmax = softmax(np.sum(predicted, axis = 1))
        result = np.where(_softmax == np.max(_softmax))
        print('Label:', labels[len(images) - target -1], ', Predicted:', result[0])

    print('-----FINISH-----')


def softmax(x):
    exp_x = np.exp(x)
    softmax_x = exp_x / np.sum(exp_x)
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
