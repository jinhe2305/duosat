package com.duosat.tv.image;

import android.graphics.Bitmap;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Paint;

import com.squareup.picasso.Transformation;

public class CircleTransform implements Transformation {
    public final static int CIRCLE_SIZE = 300;
    public final static int BORDER_SIZE = 2;

    @Override
    public Bitmap transform(Bitmap source) {
        int size = Math.min(source.getWidth(), source.getHeight());

        int x = (source.getWidth() - size) / 2;
        int y = (source.getHeight() - size) / 2;

        Bitmap squaredBitmap = Bitmap.createBitmap(source, x, y, size, size);
        if (squaredBitmap != source) {
            source.recycle();
            source = null;
        }

        source = Bitmap.createScaledBitmap(squaredBitmap, CIRCLE_SIZE, CIRCLE_SIZE, false);
        if (source != squaredBitmap) {
            squaredBitmap.recycle();
            squaredBitmap = null;
        }

        Bitmap bitmap = Bitmap.createBitmap(CIRCLE_SIZE, CIRCLE_SIZE, source.getConfig());

        Canvas canvas = new Canvas(bitmap);
        Paint paint = new Paint();
        BitmapShader shader = new BitmapShader(source, BitmapShader.TileMode.CLAMP, BitmapShader.TileMode.CLAMP);
        paint.setShader(shader);
        paint.setAntiAlias(true);

        float r = CIRCLE_SIZE/2f;
        canvas.drawCircle(r, r, r, paint);

        /*paint.setShader(null);
        paint.setColor(Color.WHITE);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(BORDER_SIZE * 2);
        canvas.drawCircle(r, r, r - BORDER_SIZE, paint);*/

        source.recycle();
        source = null;

        return bitmap;
    }

    @Override
    public String key() {
        return "circle";
    }

}
