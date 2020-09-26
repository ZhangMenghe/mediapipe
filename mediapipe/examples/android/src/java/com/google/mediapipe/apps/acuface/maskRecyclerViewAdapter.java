package com.google.mediapipe.apps.acuface;

import android.app.Activity;
import android.content.res.ColorStateList;
import android.content.res.TypedArray;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.RecyclerView;

import com.google.common.primitives.Ints;
import com.google.mediapipe.apps.basic.R;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class maskRecyclerViewAdapter extends RecyclerView.Adapter<maskRecyclerViewAdapter.MyView> {
    private final WeakReference<Activity> actRef;
    private final WeakReference<RecyclerView> recyRef;
    private List<WeakReference<Button>> buttonRefs;

    private List<String> item_names;
    private Set<String> selected_items;
    private boolean[] values;
    private int card_num = 0;

    private ColorStateList normal_color, highlight_color;

    private int mask_num, mask_bits;
    class MyView extends RecyclerView.ViewHolder {
        Button btn;
        int view_pos;
        MyView(View view, int pos) {
            super(view);
            view_pos = pos;
            btn = (Button) view.findViewById(R.id.horizontal_card_btn);
            setButtonStyle(btn, view_pos);
            btn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    int item_position = view_pos;//recyRef.get().getChildAdapterPosition((View)v.getParent());
                    values[item_position] = !values[item_position];
                    if(values[item_position])JNIInterface.JNIAddMeridian(item_names.get(item_position));//selected_items.add(item_names.get(item_position));
                    else JNIInterface.JNIDelMeridian(item_names.get(item_position));//selected_items.remove(item_names.get(item_position));
                    setButtonStyle((Button)v, item_position);

                }
            });
        }
        WeakReference<Button> getButtonRef(){return new WeakReference<>(btn);}
    }

    maskRecyclerViewAdapter(Activity activity, RecyclerView recyclerView) {
        actRef = new WeakReference<>(activity);
        recyRef = new WeakReference<>(recyclerView);
        buttonRefs = new ArrayList<>();
        selected_items = new HashSet<>();

        highlight_color = ColorStateList.valueOf(ContextCompat.getColor(activity, R.color.yellowOrange));
        normal_color = ColorStateList.valueOf(ContextCompat.getColor(activity, R.color.brightBlue));

        item_names = Arrays.asList(actRef.get().getResources().getStringArray(R.array.masks_list));
        values = new boolean[item_names.size()];
        mask_num = item_names.size() - 1;
    }

    @Override
    public MyView onCreateViewHolder(ViewGroup parent, int viewType) {
        View card_view = LayoutInflater.from(parent.getContext()).inflate(R.layout.horizontal_item, parent, false);
        card_num++;

        MyView v = new MyView(card_view, card_num-1);
        buttonRefs.add(v.getButtonRef());
        return v;
    }

    @Override
    public void onBindViewHolder(final MyView holder, final int position) {
        holder.btn.setText(item_names.get(position));
    }

    @Override
    public int getItemCount() {
        return item_names.size();
    }
    private void setButtonStyle(Button v, int pos){
        if(values[pos]){
            v.setBackgroundTintList(highlight_color);
        }else{
            v.setBackgroundTintList(normal_color);
        }
    }
    private void set_initial_values(){
        for(int i=0;i<item_names.size();i++){
            if(values[i])selected_items.add(item_names.get(i));
        }
        String[] arr = new String[selected_items.size()];
        selected_items.toArray(arr);
//        Log.e("TAG", "===set_initial_values: "+selected_items.size() );
        JNIInterface.JNIUpdateMeridianList(selected_items.size(), arr);

    }
    void Reset(){
        TypedArray tvalues = actRef.get().getResources().obtainTypedArray(R.array.masks_status);
        for(int i=0; i<item_names.size(); i++)
            values[i] = tvalues.getBoolean(i, true);
        tvalues.recycle();
        set_initial_values();

        for(int i=0; i<buttonRefs.size(); i++)
           setButtonStyle(buttonRefs.get(i).get(), i);

    }
    void Reset(boolean[] vs){
        if(vs==null || vs.length!=values.length){Reset(); return;}
        values = vs.clone();
        set_initial_values();

        for(int i=0; i<buttonRefs.size(); i++)
            setButtonStyle(buttonRefs.get(i).get(), i);

    }
    boolean[] getValues(){return values;}
//    int[] getSelections(){
//        return Ints.toArray(selected_items);
//    }
}

