package com.duosat.tv.image;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import com.squareup.picasso.Transformation;

public class SquareTransform implements Transformation {
    public static int SQUARE_WIDTH = 800;
    public final static int BORDER_SIZE = 2;

    @Override
    public Bitmap transform(Bitmap source) {
        int size = Math.min(source.getWidth(), SQUARE_WIDTH);
        double aspectRatio = (double) source.getHeight() / (double) source.getWidth();
        int targetHeight = (int) (size * aspectRatio);
        Bitmap bitmap = Bitmap.createScaledBitmap(source, size, targetHeight, false);
        if (bitmap != source) {
            source.recycle();
        }

        Bitmap result = bitmap.copy(Bitmap.Config.ARGB_8888, true);
        bitmap.recycle();

        Canvas canvas = new Canvas(result);
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setColor(Color.BLACK);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(BORDER_SIZE * 2);
        canvas.drawRect(BORDER_SIZE, BORDER_SIZE, size - BORDER_SIZE, targetHeight - BORDER_SIZE, paint);

        return result;
    }

    @Override
    public String key() {
        return "square";
    }
}
