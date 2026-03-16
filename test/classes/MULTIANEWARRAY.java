public class MULTIANEWARRAY {
    public void testMultiANewArray(int key) {
        int[][] array1 = new int[10][10];
        Integer[][] array2 = new Integer[10][10];
        Integer[] array3 = new Integer[10];
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                array1[i][j] = j;
                array2[i][j] = new Integer(j);
            }
            array3[i] = new Integer(i);
        }
    }
}
