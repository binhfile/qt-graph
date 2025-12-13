# XY Chart Widget - Test Guide: Save Graph to Image

## Quick Start

### Build the Test Application
```bash
cd C:\Users\ngoth\Downloads\files
build_test.bat
```

### Run the Test
```bash
run_test.bat
```

## Test Application Features

The test application (`test_save_graph.exe`) demonstrates and tests:

1. **Multi-Axis Graph Creation** - Three independent Y-axes with different scales
2. **Data Generation** - Automated test data generation
3. **Graph Rendering** - Visualization of three sine waves
4. **Image Export** - Save the graph as a PNG file

## Test Steps

### Step 1: Launch Application
Run `run_test.bat` and click through the instruction screen.

### Step 2: Generate Test Data
Click **"Generate Test Data"** button:
- Creates 400 data points for each of the three series
- Status bar will show: "Generated 400 points per series"
- Chart will display three sine waves with different frequencies

### Step 3: Verify Graph Display
Check that you can see:
- **Line 1 (Blue, Left Axis)**: Clear sine wave, frequency × 0.1, scale -1.2 to 1.2
- **Line 2 (Red, Right Axis)**: Sine wave, frequency × 0.2, scale -150 to 150
- **Line 3 (Green, Left Axis)**: Sine wave, frequency × 0.15, scale -1,000,000 to 1,000,000

### Step 4: Save Graph to Image
Click **"Save Graph to Image"** button:
- Creates PNG file with timestamp in filename
- Status bar shows save location and result
- Files saved to: `C:\Users\ngoth\Downloads\files\test_graph_YYYY-MM-DD_hh-mm-ss.png`

### Step 5: Verify Saved File
Open File Explorer and navigate to `C:\Users\ngoth\Downloads\files\`:
- Look for files named `test_graph_*.png`
- Open the file with an image viewer
- Verify the image contains:
  - Title: "Test Graph - Multi-Axis Sine Waves"
  - Three colored lines (blue, red, green)
  - Axis labels
  - Grid lines
  - Legend

### Step 6: Test Multiple Saves
Click "Generate Test Data" and "Save Graph to Image" multiple times:
- Verify each file gets a unique timestamp
- Verify all files are created successfully
- Confirm status messages appear for each save

## Test Requirements Checklist

✅ **Build Requirements**
- [ ] `build_test.bat` runs successfully
- [ ] No compilation errors
- [ ] Executable created: `test_save_graph.exe`

✅ **Functionality Requirements**
- [ ] Application launches without errors
- [ ] Chart displays correctly
- [ ] Three Y-axes visible with different scales
- [ ] Three sine waves render correctly
- [ ] "Generate Test Data" button works
- [ ] "Save Graph to Image" button works
- [ ] PNG files created successfully

✅ **File Requirements**
- [ ] Files saved with correct naming convention
- [ ] Files saved in correct location
- [ ] Files are valid PNG images
- [ ] File size > 100 KB

✅ **Image Content Requirements**
- [ ] Title text visible: "Test Graph - Multi-Axis Sine Waves"
- [ ] Three colored sine wave lines visible
- [ ] Blue line (Line 1) clear and distinct
- [ ] Red line (Line 2) visible
- [ ] Green line (Line 3) visible
- [ ] Axis labels present
- [ ] Grid lines visible
- [ ] Chart background white

✅ **UI/UX Requirements**
- [ ] Status bar updates with messages
- [ ] Buttons are clickable
- [ ] Application remains responsive
- [ ] No crashes or freezes
- [ ] Clear button clears the chart
- [ ] Exit button closes application

## Expected Test Results

### Successful Test Session
```
Status: Generated 400 points per series
-> All three sine waves visible on chart

Status: Graph saved successfully to test_graph_2025-12-13_14-30-45.png
-> PNG file created in correct location
-> Image contains all chart elements
-> File is valid and can be opened in image viewer
```

### File Output
```
C:\Users\ngoth\Downloads\files\test_graph_2025-12-13_14-30-45.png  (~150 KB)
C:\Users\ngoth\Downloads\files\test_graph_2025-12-13_14-31-12.png  (~150 KB)
C:\Users\ngoth\Downloads\files\test_graph_2025-12-13_14-31-38.png  (~150 KB)
```

## Troubleshooting

### Issue: "Test executable not found"
**Solution:** Run `build_test.bat` first to build the application

### Issue: Graph appears blank
**Solution:** Click "Generate Test Data" to populate the chart with test data

### Issue: Cannot save graph
**Solution:**
- Verify write permissions to `C:\Users\ngoth\Downloads\files\`
- Check available disk space
- Try a different location if path issues occur

### Issue: Saved image is blank/black
**Solution:**
- This may be a rendering issue
- Try generating new data and saving again
- Verify application window is fully visible

### Issue: Application crashes
**Solution:**
- Close any other applications
- Restart the test application
- Check Windows Event Viewer for error details

## Advanced Testing

### Test Multiple Saves
```bash
run_test.bat
```
1. Click "Generate Test Data"
2. Click "Save Graph to Image"
3. Click "Generate Test Data" again
4. Click "Save Graph to Image" (should create new file with different timestamp)
5. Verify both files exist and have correct timestamps

### Test Clear Functionality
```bash
run_test.bat
```
1. Click "Generate Test Data" (chart fills with data)
2. Click "Clear" (chart should empty)
3. Click "Generate Test Data" again (chart refills)
4. Verify chart state changes correctly

### Test Legend
```bash
run_test.bat
```
1. Click "Generate Test Data"
2. Look for legend showing:
   - Line 1 (small) - Blue
   - Line 2 (medium) - Red
   - Line 3 (large) - Green

## What Gets Tested

### Feature: Multi-Axis Support
- ✅ Three independent Y-axes created
- ✅ Each axis has its own scale
- ✅ Series assigned to correct axes
- ✅ Different value ranges handled correctly

### Feature: Graph Rendering
- ✅ All three sine waves render correctly
- ✅ Colors are distinct (blue, red, green)
- ✅ Axes, labels, and grid visible
- ✅ Title and legends display

### Feature: Image Export
- ✅ Pixmap creation from widget
- ✅ PNG file format
- ✅ Timestamp in filename
- ✅ File saved to correct location

### Feature: Data Management
- ✅ Multiple data points handled
- ✅ Series tracking with yAxisId
- ✅ Max points per series limit (500)
- ✅ Clear functionality works

## Performance Metrics

**Expected Performance:**
- Application startup: < 2 seconds
- Generate 400 points: < 1 second
- Save to image: < 500 ms
- Memory usage: < 100 MB

## Verification Criteria (PASS/FAIL)

**PASS Criteria:**
- ✅ All tests complete without errors
- ✅ All PNG files created successfully
- ✅ Saved images contain all expected elements
- ✅ Files are valid and can be opened
- ✅ No application crashes

**FAIL Criteria:**
- ❌ Any compilation errors
- ❌ Application crashes
- ❌ Images not saved
- ❌ Saved images are blank/corrupted
- ❌ Missing chart elements in saved image

## Test Report Location

Generate test report: `C:\Users\ngoth\.claude\plans\test-requirements.md`

Fill in the test results template and save for documentation.
