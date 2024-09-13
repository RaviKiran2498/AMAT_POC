# lib pip install openpyxl
# lib pip install pandas
import pandas as pd
import numpy as np
import warnings
import argparse

class MetaDataExtraction:
    def __init__(self):
        # self.cad_df = pd.read_csv(r"TempData\get_File_And_Related_BOMDetails1.csv")
        # self.erp_df = pd.read_excel(r"TempData\ERP_Data_2024.xlsx", sheet_name=None)
        self.cad_df = None
        self.erp_df = None
    
    def getFileBomDetails(self,erp_df,nxprop_df,bom_merge_save,type_count_save):
        # Load CAD CSV data into DataFrame
        cad_df = self.cad_df
        if 'ParentRev' not in cad_df.columns:
            cad_df['ParentRev'] = None
        
        if 'ChildRev' not in cad_df.columns:
            cad_df['ChildRev'] = None
        
        cad_df = cad_df.add_suffix('_cad')
        erp_df = erp_df.add_suffix('_erp')
        def checkRow(erp_df,cad_df):
            unique_parents = set(erp_df['ImmediateParent_erp'].unique())
            unique_children = set(erp_df['No._erp'].unique())
            combined_unique_values1 = unique_parents.union(unique_children)


            unique_parents = set(cad_df['Parent Name_cad'].unique())
            unique_children = set(cad_df['Child Name_cad'].unique())
            combined_unique_values2 = unique_parents.union(unique_children)

            # Define the match criteria function
            def match_criteria(erp_value, cad_value):
                if erp_value + "_asm" == cad_value:
                    return True
                elif erp_value == cad_value:
                    return True
                elif erp_value + "-001_asm" == cad_value:
                    return True
                elif erp_value + "-001" == cad_value:
                    return True
                if cad_value.startswith(erp_value) and cad_value.endswith("_asm"):
                    return True
                elif cad_value.startswith(erp_value):
                    return True
                else:
                    return False

            # Dictionary to store matches
            matches = {}
            for erp_value in combined_unique_values1:
                matches[erp_value] = [cad_value for cad_value in combined_unique_values2 if match_criteria(erp_value, cad_value)]

            priority_matches = {}
            for erp_value, cad_values in matches.items():
                if not cad_values:
                    priority_matches[erp_value] = None
                else:
                    if erp_value + "_asm" in cad_values:
                        priority_matches[erp_value] = erp_value + "_asm"
                    elif erp_value in cad_values:
                        priority_matches[erp_value] = erp_value
                    elif erp_value + "-001_asm" in cad_values:
                        priority_matches[erp_value] = erp_value + "-001_asm"
                    elif erp_value + "-001" in cad_values:
                        priority_matches[erp_value] = erp_value + "-001"
                    elif any(cad_value.startswith(erp_value) and cad_value.endswith("_asm") for cad_value in cad_values):
                        priority_matches[erp_value] = next(cad_value for cad_value in cad_values if cad_value.startswith(erp_value) and cad_value.endswith("_asm"))
                    elif any(cad_value.startswith(erp_value) for cad_value in cad_values):
                        priority_matches[erp_value] = next(cad_value for cad_value in cad_values if cad_value.startswith(erp_value))
                    else:
                        priority_matches[erp_value] = None

            # Function to update values based on matches dictionary
            def update_value(value):
                return priority_matches.get(value, value)

            erp_df['UpdatedImmediateParent_erp'] = None
            erp_df['UpdatedChildName_erp'] = None
            # Update the 'ImmediateParentName' and 'ChildName' columns based on the matches dictionary
            erp_df['UpdatedImmediateParent_erp'] = erp_df['ImmediateParent_erp'].apply(update_value)
            erp_df['UpdatedChildName_erp'] = erp_df['No._erp'].apply(update_value)

            matched_df = pd.merge(
                erp_df,
                cad_df,
                how='inner',
                left_on=['UpdatedImmediateParent_erp', 'UpdatedChildName_erp'],
                right_on=['Parent Name_cad', 'Child Name_cad']
            )
            merged_df2 = pd.merge(
            erp_df,
            cad_df,
            how='left',
            left_on=['UpdatedImmediateParent_erp', 'UpdatedChildName_erp'],
            right_on=['Parent Name_cad', 'Child Name_cad']
            )
            merged_df3 = pd.merge(
                erp_df,
                cad_df,
                how='right',
                left_on=['UpdatedImmediateParent_erp', 'UpdatedChildName_erp'],
                right_on=['Parent Name_cad', 'Child Name_cad']
            )
            # Concatenate DataFrames
            concat_df = pd.concat([matched_df, merged_df3])
            unique_Cad_df = concat_df.drop_duplicates(keep=False)

            concat_df = pd.concat([matched_df, merged_df2])
            unique_erp_df = concat_df.drop_duplicates(keep=False)

            return [matched_df,unique_Cad_df,unique_erp_df]
        
        matched_df,unique_in_cad,unique_in_erp = checkRow(erp_df,cad_df)
        matched_df = matched_df.drop_duplicates(subset='Transformation Matrix_cad', keep='first')
        print(matched_df)
        print(unique_in_cad)
        print(unique_in_erp)
        
        # Replace invalid revision values and handle CAD revision data
        matched_df["ImmediateParentRev_erp"] = matched_df["ImmediateParentRev_erp"].replace(["NA", "#N/A"], "")
        matched_df["Part Rev_erp"] = matched_df["Part Rev_erp"].replace(["NA", "#N/A"], "")

        # Determine final parent and child revision
        matched_df['FinalParentRev'] = np.where(matched_df["ImmediateParentRev_erp"].isnull(),
                                                matched_df["ParentRev_cad"], matched_df["ImmediateParentRev_erp"])
        matched_df['FinalChildRev'] = np.where(matched_df["Part Rev_erp"].isnull(),
                                            matched_df["ChildRev_cad"], matched_df["Part Rev_erp"])
        matched_df['isMaster'] = 'Yes'
        matched_df['ERPGo'] = 'Go'

        #Adding additional data to aligin with matched data
        if unique_in_cad is not None and not unique_in_cad.empty:
            unique_in_cad = unique_in_cad.copy()
            unique_in_cad.loc[:, 'isMaster'] = 'Yes'
            unique_in_cad.loc[:, 'ERPGo'] = None
            unique_in_cad.loc[:, 'ParentIndex_erp'] = -1
            unique_in_cad.loc[:, 'ChildIndex_erp'] = -1
            unique_in_cad['FinalParentRev'] = unique_in_cad["ParentRev_cad"]
            unique_in_cad['FinalChildRev']  = unique_in_cad["ChildRev_cad"]

        if unique_in_erp is not None and not unique_in_erp.empty:
            unique_in_erp = unique_in_erp.copy()
            unique_in_erp.loc[:, 'isMaster'] = None
            unique_in_erp.loc[:, 'ERPGo'] = 'Go'
            unique_in_erp.loc[:, 'Matched_cad'] = False
            unique_in_erp['FinalParentRev'] = unique_in_erp["ImmediateParentRev_erp"].fillna("")
            unique_in_erp['FinalChildRev']  = unique_in_erp["Part Rev_erp"].fillna("")

        unique_in_erp['EXTRA Child Name'] = unique_in_erp["No._erp"]
        match_list_columns = ["Parent Name_cad","Child Name_cad","No._erp","Transformation Matrix_cad","Quantity_cad","FinalParentRev","FinalChildRev","isMaster","ERPGo"]
        erp_list_columns = ["ImmediateParent_erp","No._erp","EXTRA Child Name","Transformation Matrix_erp","Quantity_erp","FinalParentRev","FinalChildRev","isMaster","ERPGo","Object Type"]
        cad_list_columns = ["Parent Name_cad","Child Name_cad","No._erp","Transformation Matrix_cad","Quantity_cad","FinalParentRev","FinalChildRev","isMaster","ERPGo"]
        final_list_columns = ['Parent Name',"Child Name","Child Name ERP",'Transformation Matrix','Quantity',"Parent Rev","Child Rev","isMaster","ERPGo"]
        
        new_matched_df = pd.DataFrame()
        unique_in_cad2 = pd.DataFrame()
        unique_in_erp2 = pd.DataFrame()

        if not matched_df.empty:
            new_matched_df = matched_df[match_list_columns]
            
            rename_dict1 = dict(zip(match_list_columns, final_list_columns))
            new_matched_df = new_matched_df.rename(columns=rename_dict1)
        else:
            print("matched_df is empty or missing necessary columns, skipping.")

        # Check if 'unique_in_cad' has data before processing
        if not unique_in_cad.empty:
            unique_in_cad2 = unique_in_cad[cad_list_columns]
            rename_dict2 = dict(zip(cad_list_columns, final_list_columns))
            unique_in_cad2 = unique_in_cad2.rename(columns=rename_dict2)
        else:
            print("unique_in_cad is empty or missing necessary columns, skipping.")

        # Check if 'unique_in_erp' has data before processing
        if not unique_in_erp.empty:
            unique_in_erp["Transformation Matrix_erp"] = None
            unique_in_erp["Child Name_erp"] = None
            unique_in_erp["Parent Name_erp"] = None
            unique_in_erp['Object Type'] = None
            
            unique_in_erp2 = unique_in_erp[erp_list_columns]
            column1_values = set(unique_in_erp2['ImmediateParent_erp'])
            print(unique_in_erp2)
            
            # Create 'Object Type' based on whether 'No._erp' values exist in 'ImmediateParent_erp'
            unique_in_erp2['Object Type'] = unique_in_erp2['No._erp'].apply(lambda x: 'Assembly_Kit' if x in column1_values else 'Mech_Design')
            
            rename_dict3 = dict(zip(erp_list_columns, final_list_columns))
            unique_in_erp2 = unique_in_erp2.rename(columns=rename_dict3)
        else:
            print("unique_in_erp is empty or missing necessary columns, skipping.")

        df_concat = pd.concat([new_matched_df, unique_in_cad2,unique_in_erp2], axis=0, ignore_index=True)
        df_concat['Child Name ERP'] = df_concat['Child Name ERP'].replace('', np.nan)  # Ensure empty strings are treated as NaN
        df_concat['Child Name ERP'] = df_concat['Child Name ERP'].fillna(df_concat['Child Name'])
        df_concat = df_concat.sort_values(by='Parent Name')
        # generate the sequence number
        df_concat['Seq.No'] = None
        df_concat['parent_child'] = df_concat['Parent Name'] + '_' + df_concat['Child Name']
        df_concat['Seq.No'] = df_concat.groupby('Parent Name')['parent_child'].transform(lambda x: pd.factorize(x)[0] + 1) * 10
        # Drop the helper column 'parent_child'
        df_concat = df_concat.drop(columns=['parent_child'])

        # # get the topline
        x = erp_df['Production BOM No._erp'].drop_duplicates().tolist()
        if len(x) > 0:
            new_rows = pd.DataFrame({
                'Parent Name': [''] * len(x),  # List of empty strings
                'Child Name': x,
                'Object Type': ['Assembly_Kit'] * len(x) , # List of 'Assembly_Kit'
                'Child Name ERP': x * len(x)
            })

            # Concatenate the new rows to the top of the original DataFrame
            df_concat = pd.concat([new_rows, df_concat], ignore_index=True)

        # # # Identify Parent Names that are not in Child Names
        # missing_parents = df_concat['Parent Name'][~df_concat['Parent Name'].isin(df_concat['Child Name'])]

        # # Create new rows for missing parents
        # new_rows = pd.DataFrame({
        #     'Parent Name': '',
        #     'Child Name': missing_parents,
        #     'Object Type': 'Assembly_Kit',
        #     'Child Name_cad': missing_parents + '-001_asm'
        # })
        # new_rows = new_rows.drop_duplicates(subset='Child Name')

        # # Concatenate the new rows to the top of the original DataFrame
        # df_concat = pd.concat([new_rows, df_concat], ignore_index=True)
        # df_concat = df_concat.drop(columns=['Child Name_cad'])

        #  merge NX Prop
        df_concat = (df_concat.merge(nxprop_df, left_on='Child Name', right_on='File Name', how='left'))
        # Update the Child rev if not exist revision
        def update_child_rev(row):
            if pd.isna(row['Child Rev']) or row['Child Rev'].strip() == '':
                # If 'REVISION' is NaN or 'ANY', set 'Child Rev' to '01'
                if pd.isna(row['REVISION']) or row['REVISION'] == 'ANY':
                    return '01'
                # Otherwise, set 'Child Rev' to the value in 'REVISION'
                else:
                    return row['REVISION']
            # If 'Child Rev' is already set, return its value
            return row['Child Rev']
        # Apply the logic to update the 'Child Rev' column
        df_concat['Child Rev'] = df_concat.apply(update_child_rev, axis=1)
        
        # Based on description "screw" change object type to OEM
        # Define the logic for the 'Object Type' column
        def determine_object_type(file_name,description):
            # Check if the description contains 'screw' anywhere in the string
            if isinstance(description, str) and 'screw' in description.lower():
                return 'OEMItem'
            if not isinstance(file_name, str) or file_name.strip() == '':
                return ''  # Return empty string if file_name is empty or invalid
            if file_name.endswith('.iam'):
                return 'Assembly_Kit'
            elif file_name.endswith('.ipt'):
                return 'Mech_Design'
            else:
                return ''
        
        if 'Object Type' not in df_concat.columns:
            df_concat['Object Type'] = None
        # Apply the logic to create the 'Object Type' column
        df_concat['Object Type'] = df_concat.apply(
            lambda row: row['Object Type'] if pd.notna(row['Object Type']) and row['Object Type'].strip() != '' 
                        else determine_object_type(row['SourceCADFile'], row['DB_PART_DESC']),
            axis=1
        )
        # Based on description "screw" change object type to OEM

        unique_df = df_concat.drop_duplicates(subset='Child Name')
        # Step 2: Get the count of each 'Object Type' value
        object_type_counts = unique_df['Object Type'].value_counts()
        # Convert to dictionary
        object_type_count_dict = object_type_counts.to_dict()

        df_concat = df_concat.rename(columns={'Child Rev': 'Item Rev'})
        df_concat['Parent Item Type'] = 'Assembly_Kit'
        df_concat['ItemID'] = None
        df_concat['DatasetType'] = None
        df_concat['Parent Item ID'] = None

        print(df_concat)
        # Create a csv which stores the count of type
        typeCount_df = pd.DataFrame(list(object_type_count_dict.items()), columns=['Type', 'Count'])
        typeCount_df = typeCount_df[typeCount_df['Type'] != '']
        typeCount_df.to_csv(type_count_save, index=False,sep='|')
        df_concat.to_csv(bom_merge_save, index=False,sep='|')
    
    def updateErpData(self,erp_df):
        erp_df.columns = erp_df.columns.str.strip()

        # Initialize empty columns for Immediate Parent and Immediate Parent Revision
        erp_df.loc[:,'ImmediateParent'] = None
        erp_df.loc[:,'ImmediateParentRev'] = None

        # Dictionary to track the most recent parent and its revision at each level
        parent_map = {}

        # Iterate through the rows to determine the Immediate Parent and its revision
        for index, row in erp_df.iterrows():
            top_line = row['Production BOM No.']
            level = row['Level']
            child = row['No.']
            child_rev = row['Part Rev']

            # For level 1, the Immediate Parent is the Top line itself and revision is None
            if level == 1:
                erp_df.at[index, 'ImmediateParent'] = top_line
                erp_df.at[index, 'ImmediateParentRev'] = None  # Keep revision as None for the Top line
                # Reset the parent map for level 1
                parent_map[top_line] = {level: (child, child_rev)}
            else:
                if top_line in parent_map and (level - 1) in parent_map[top_line]:
                    erp_df.at[index, 'ImmediateParent'] = parent_map[top_line][level - 1][0]
                    erp_df.at[index, 'ImmediateParentRev'] = parent_map[top_line][level - 1][1]
                else:
                    erp_df.at[index, 'ImmediateParent'] = None
                    erp_df.at[index, 'ImmediateParentRev'] = None

                if top_line not in parent_map:
                    parent_map[top_line] = {}
                parent_map[top_line][level] = (child, child_rev)

        # Save the result to a new CSV file if needed
        # erp_df.to_csv("TempData/test_erp_data_10SEP2024_new.csv", index=False)
        return erp_df
    
    def getIdBasedDataFromERP(self,column_name,topline_id):
        # Initialize a variable to hold the processed DataFrame
        processed_df = pd.DataFrame()
        sheets = self.erp_df
        # Iterate through each sheet and process the DataFrames
        for sheet_name, df in sheets.items():
            df.columns = df.columns.str.strip()
            if column_name in df.columns:
                # Strip leading/trailing whitespace from the column
                df[column_name] = df[column_name].astype(str).str.strip()
                # Filter rows where the column matches the search string
                processed_df = df[df[column_name] == topline_id]
                if not processed_df.empty:
                    break
        # print("processed_df: ",processed_df)
        return processed_df

#---------Code Starts Here--------------
if __name__ == "__main__":
    warnings.filterwarnings("ignore")
    ME = MetaDataExtraction()
    # column_name = str(input("Enter Column Name: "))
    # topline_id  = str(input("Enter Topline Name: "))
    # erp_file_path  = str(input("Enter ERP File Path: "))
    # cad_file_path  = str(input("Enter Cad File Path: "))


    parser = argparse.ArgumentParser(
        description="Test Bom Merge exe"
    )
    parser.add_argument('-c', '--columnname', type=str, required=True,
                        help="Give erp column name")
    parser.add_argument('-t', '--toplinename', type=str, required=True,
                        help="First argument to be written to the file.")
    parser.add_argument('-e', '--erpfile', type=str, required=True,
                        help="Second argument to be written to the file.")
    parser.add_argument('-b', '--cadfile', type=str, required=True,
                        help="Third argument to be written to the file.")
    parser.add_argument('-n', '--nxpropfile', type=str, required=True,
                        help="Fourth argument to be written to the file.")
    parser.add_argument('-bo', '--bommerged', type=str, required=True,
                        help="Fifth argument to be written to the file.")
    parser.add_argument('-tc', '--typecount', type=str, required=True,
                        help="Fifth argument to be written to the file.")

    args = parser.parse_args()
    column_name = args.columnname
    topline_id = args.toplinename
    erp_file_path = args.erpfile
    cad_file_path = args.cadfile
    nx_file_path = args.nxpropfile
    latest_bom_merge = args.bommerged
    type_count = args.typecount
    drw_file_path = args.drwFile

    ME.erp_df = pd.read_excel(erp_file_path, sheet_name=None)
    ME.cad_df = pd.read_csv(cad_file_path, sep=',')
    nx_file_df = pd.read_csv(nx_file_path, sep=',')
    drw_file_df = pd.read_csv(drw_file_path,sep=',')

    topline_id = topline_id[:10]
    Id_based_erp_df = ME.getIdBasedDataFromERP(column_name,topline_id)
    if not Id_based_erp_df.empty:
        update_erp_df = ME.updateErpData(Id_based_erp_df)
        if not update_erp_df.empty:
            extracted_df = ME.getFileBomDetails(update_erp_df,nx_file_df,latest_bom_merge,type_count,drw_file_df)
            print(f"DF is saved Successfully at location {latest_bom_merge}")
    else:
        print("Top Line Not Found")